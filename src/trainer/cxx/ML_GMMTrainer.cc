/**
 * @file trainer/cxx/ML_GMMTrainer.cc
 * @date Tue May 10 11:35:58 2011 +0200
 * @author Francois Moulin <Francois.Moulin@idiap.ch>
 *
 * Copyright (C) 2011-2013 Idiap Research Institute, Martigny, Switzerland
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <bob/trainer/ML_GMMTrainer.h>
#include <algorithm>

bob::trainer::ML_GMMTrainer::ML_GMMTrainer(const bool update_means,
    const bool update_variances, const bool update_weights,
    const double mean_var_update_responsibilities_threshold): 
  bob::trainer::GMMTrainer(update_means, update_variances, update_weights, 
    mean_var_update_responsibilities_threshold)
{
}

bob::trainer::ML_GMMTrainer::ML_GMMTrainer(const bob::trainer::ML_GMMTrainer& b):
  bob::trainer::GMMTrainer(b)
{
}

bob::trainer::ML_GMMTrainer::~ML_GMMTrainer()
{  
}

void bob::trainer::ML_GMMTrainer::initialization(bob::machine::GMMMachine& gmm,
  const blitz::Array<double,2>& data)
{
  bob::trainer::GMMTrainer::initialization(gmm, data);
  // Allocate cache
  size_t n_gaussians = gmm.getNGaussians();
  m_cache_ss_n_thresholded.resize(n_gaussians);
}


void bob::trainer::ML_GMMTrainer::mStep(bob::machine::GMMMachine& gmm,
  const blitz::Array<double,2>& data)
{
  // Read options and variables
  const size_t n_gaussians = gmm.getNGaussians();

  // - Update weights if requested
  //   Equation 9.26 of Bishop, "Pattern recognition and machine learning", 2006
  if (m_update_weights) {
    blitz::Array<double,1>& weights = gmm.updateWeights();
    weights = m_ss.n / static_cast<double>(m_ss.T); //cast req. for linux/32-bits & osx
    // Recompute the log weights in the cache of the GMMMachine
    gmm.recomputeLogWeights();
  }

  // Generate a thresholded version of m_ss.n
  for(size_t i=0; i<n_gaussians; ++i)
    m_cache_ss_n_thresholded(i) = std::max(m_ss.n(i), m_mean_var_update_responsibilities_threshold);  

  // Update GMM parameters using the sufficient statistics (m_ss)
  // - Update means if requested
  //   Equation 9.24 of Bishop, "Pattern recognition and machine learning", 2006
  if (m_update_means) {
    for(size_t i=0; i<n_gaussians; ++i) {
      blitz::Array<double,1>& means = gmm.updateGaussian(i)->updateMean();
      means = m_ss.sumPx(i, blitz::Range::all()) / m_cache_ss_n_thresholded(i);
    }   
  }

  // - Update variance if requested
  //   See Equation 9.25 of Bishop, "Pattern recognition and machine learning", 2006
  //   ...but we use the "computational formula for the variance", i.e.
  //   var = 1/n * sum (P(x-mean)(x-mean))
  //       = 1/n * sum (Pxx) - mean^2
  if (m_update_variances) {
    for(size_t i=0; i<n_gaussians; ++i) {
      const blitz::Array<double,1>& means = gmm.getGaussian(i)->getMean();
      blitz::Array<double,1>& variances = gmm.updateGaussian(i)->updateVariance();
      variances = m_ss.sumPxx(i, blitz::Range::all()) / m_cache_ss_n_thresholded(i) - blitz::pow2(means);
      gmm.updateGaussian(i)->applyVarianceThresholds();
    }
  }
}

bob::trainer::ML_GMMTrainer& bob::trainer::ML_GMMTrainer::operator=
  (const bob::trainer::ML_GMMTrainer &other)
{
  if (this != &other)
  {
    bob::trainer::GMMTrainer::operator=(other);
    m_cache_ss_n_thresholded.resize(other.m_cache_ss_n_thresholded.extent(0));
  }
  return *this;
}

bool bob::trainer::ML_GMMTrainer::operator==
  (const bob::trainer::ML_GMMTrainer &other) const
{
  return bob::trainer::GMMTrainer::operator==(other);
}

bool bob::trainer::ML_GMMTrainer::operator!=
  (const bob::trainer::ML_GMMTrainer &other) const
{
  return !(this->operator==(other));
}

bool bob::trainer::ML_GMMTrainer::is_similar_to
  (const bob::trainer::ML_GMMTrainer &other, const double r_epsilon, 
   const double a_epsilon) const
{
  return bob::trainer::GMMTrainer::is_similar_to(other, r_epsilon, a_epsilon);
}


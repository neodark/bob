/**
 * @author <a href="mailto:andre.dos.anjos@gmail.com">Andre Anjos</a> 
 * @date Mon  7 Feb 21:34:22 2011 
 *
 * @brief Implements the BinaryArrayCodec type 
 */

#include "io/BinaryArrayCodec.h"
#include "io/BinFile.h"
#include "io/ArrayCodecRegistry.h"
#include "io/Exception.h"

namespace io = Torch::io;

//Takes care of the codec registration.
static bool register_codec() {
  io::ArrayCodecRegistry::addCodec(boost::shared_ptr<io::ArrayCodec>(new io::BinaryArrayCodec())); 
  return true;
}

static bool codec_registered = register_codec(); 

io::BinaryArrayCodec::BinaryArrayCodec()
  : m_name("torch.array.binary"),
    m_extensions()
{ 
  m_extensions.push_back(".bin");
}

io::BinaryArrayCodec::~BinaryArrayCodec() { }

void io::BinaryArrayCodec::peek(const std::string& filename, 
    Torch::core::array::ElementType& eltype, size_t& ndim,
    size_t* shape) const {
  io::BinFile f(filename, io::BinFile::in);
  if (!f) {
    eltype = Torch::core::array::t_unknown;
    ndim = 0;
    throw io::FileNotReadable(filename);
  }
  eltype = f.getElementType();
  ndim = f.getNDimensions();
  for (size_t i=0; i<ndim; ++i) shape[i] = f.getShape()[i]; 
}

io::detail::InlinedArrayImpl 
io::BinaryArrayCodec::load(const std::string& filename) const {
  io::BinFile f(filename, io::BinFile::in);
  if (!f) throw io::FileNotReadable(filename);
  return f.read();
}

void io::BinaryArrayCodec::save (const std::string& filename, 
    const io::detail::InlinedArrayImpl& data) const {
  io::BinFile f(filename, io::BinFile::out);
  f.write(data);
}
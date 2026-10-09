#pragma once

#include <string>

class Texture {
public:
  Texture(const std::string &path);
  Texture(unsigned char r, unsigned char g, unsigned char b);

  unsigned int getTextureID() const;

private:
  unsigned int id;
};

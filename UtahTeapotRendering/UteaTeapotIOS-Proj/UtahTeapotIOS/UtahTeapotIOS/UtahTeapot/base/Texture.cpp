//
//  Texture.cpp
//  TexturedUtahTeapotIOS
//
//  Created by SeanRen on 2020/3/11.
//  Copyright © 2020 SeanRen. All rights reserved.
//

#include "Texture.h"

#define STBI_NO_THREAD_LOCALS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



/**
* Cubemap and Texture2d implementations for Class Texture.
*/
class TextureCubemap :public Texture {
protected:
    GLuint texId_ = GL_INVALID_VALUE;
    bool activated_ = false;
    
public:
    virtual ~TextureCubemap();
    TextureCubemap(std::vector<std::string>& texFiles);
    virtual bool GetActiveSamplerInfo(std::vector<std::string>& names,
                                      std::vector<GLint>& units);
    virtual bool Activate(void);
    virtual GLuint GetTexType();
    virtual GLuint GetTexId();
};

class Texture2d :public Texture {
protected:
    GLuint texId_ = GL_INVALID_VALUE;
    bool activated_ = false;
public:
    virtual ~Texture2d();
    // Implement just one texture
    Texture2d(std::string &texFiles);
    virtual bool GetActiveSamplerInfo(std::vector<std::string>& names,
                                      std::vector<GLint>& units);
    virtual bool Activate(void);
    virtual GLuint GetTexType();
    virtual GLuint GetTexId();
};

/**
 * Capability debug string
 */
static const std::string supportedTextureTypes = "GL_TEXTURE_2D(0x0DE1) GL_TEXTURE_CUBE_MAP(0x8513)";


/**
 * Interface implementations
 */
Texture::Texture() {}
Texture::~Texture() {}
/**
 * Create Texture Object
 * @param texFiles holds the texture file name(s) under APK's assets
 * @param type should be one (GL_TEXTURE_2D / GL_TEXTURE_CUBE_MAP)
 * @return is the newly created Texture Object
 */
Texture* Texture::Create( GLuint type, std::vector<std::string>& texFiles) {
    if (type == GL_TEXTURE_2D) {
        return dynamic_cast<Texture*>(new Texture2d(texFiles[0]));
    } else if (type == GL_TEXTURE_CUBE_MAP) {
        return dynamic_cast<Texture*>(new TextureCubemap(texFiles));
    }
    
    printf("Unknow texture type %x to created", type);
    printf("Supported Texture Types: %s", supportedTextureTypes.c_str());
    assert(false);
    return nullptr;
}

void Texture::Delete(Texture* obj) {
    if(obj == nullptr) {
        //ASSERT(false, "NULL pointer to Texture::Delete() function");
        return;
    }
    
    GLuint type = obj->GetTexType();
    if(type == GL_TEXTURE_2D) {
        Texture2d *d2Instance = dynamic_cast<Texture2d*>(obj);
        if (d2Instance) {
            delete d2Instance;
        } else {
            //ASSERT(false, "Unknown obj type to %s", __FUNCTION__);
        }
    } else if(type == GL_TEXTURE_CUBE_MAP) {
        TextureCubemap *cubemapInstance = dynamic_cast<TextureCubemap*>(obj);
        if (cubemapInstance) {
            delete cubemapInstance;
        } else {
            //ASSERT(false, "Unknown obj type to %s", __FUNCTION__);
        }
    } else {
        printf("Supported Texture Types: %s", supportedTextureTypes.c_str());
        //ASSERT(false, "Unknow texture type %x to delete", type);
    }
}

/**
 * TextureCubemap implementations
 */
bool TextureCubemap::Activate(void) {
    assert(texId_ != GL_INVALID_VALUE);
    
    glBindTexture(texId_, GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE0 + 0);
    activated_ = true;
    return true;
}

GLuint TextureCubemap::GetTexType() {
    return GL_TEXTURE_CUBE_MAP;
}

GLuint TextureCubemap::GetTexId() {
    assert(texId_ != GL_INVALID_VALUE);
    return texId_;
}

TextureCubemap::TextureCubemap(std::vector<std::string> &files) {
    // For Cubemap, we use world normal to sample the textures
    // so no texture vbo necessary
    
    int32_t imgWidth, imgHeight, channelCount;
    
    glGenTextures(1, &texId_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texId_);
    
    if(texId_ == GL_INVALID_VALUE) {
        assert(false);
        return;
    }
    
    for(GLuint i = 0; i < 6; i++) {
        
        // tga/bmp files are saved as vertical mirror images ( at least more than half ).
        stbi_set_flip_vertically_on_load(1);
        
//        uint8_t* imageBits = stbi_load_from_memory(
//                                                   fileBits.data(), fileBits.size(),
//                                                   &imgWidth, &imgHeight, &channelCount, 4);
        
        
        uint8_t* imageBits = stbi_load(files[i].c_str(), &imgWidth, &imgHeight, &channelCount,0);
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGBA,
                     imgWidth, imgHeight,
                     0, GL_RGBA,
                     GL_UNSIGNED_BYTE, imageBits);
        stbi_image_free(imageBits);
    }
    
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT );
    
    glActiveTexture(GL_TEXTURE0);
    activated_ = true;
}

/**
 * Dtor
 *    clean-up function
 */
TextureCubemap::~TextureCubemap() {
    if (texId_!= GL_INVALID_VALUE) {
        glDeleteTextures(1, &texId_);
        texId_ = GL_INVALID_VALUE;
        activated_ =  false;
    }
}

/**
 Return used sampler names and units
 so application could configure shader's sampler uniform(s).
 Cubemap just used one sampler at unit 0 with "samplerObj" as its name.
 */
bool TextureCubemap::GetActiveSamplerInfo(std::vector<std::string>& names,
                                          std::vector<GLint>& units) {
    names.clear();
    names.push_back(std::string("samplerObj"));
    units.clear();
    units.push_back(0);
    
    return true;
}

/**
 * Texture2D implementation
 */
Texture2d::Texture2d(std::string& fileName)  {
    
    int32_t imgWidth, imgHeight, channelCount;
    std::string texName(fileName);
    
    glGenTextures(1, &texId_);
    glBindTexture(GL_TEXTURE_2D, texId_);
    
    if(texId_ == GL_INVALID_VALUE) {
        assert(false);
        return;
    }
    
    
    // tga/bmp files are saved as vertical mirror images ( at least more than half ).
    stbi_set_flip_vertically_on_load(1);
    
    uint8_t* imageBits = stbi_load(fileName.c_str(), &imgWidth, &imgHeight, &channelCount,0);
    
    glTexImage2D(GL_TEXTURE_2D, 0,  // mip level
                 GL_RGBA,
                 imgWidth, imgHeight,
                 0,                // border color
                 GL_RGBA, GL_UNSIGNED_BYTE, imageBits);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    
    glActiveTexture(GL_TEXTURE0);
    
    stbi_image_free(imageBits);
}

Texture2d::~Texture2d() {
    if (texId_!= GL_INVALID_VALUE) {
        glDeleteTextures(1, &texId_);
        texId_ = GL_INVALID_VALUE;
    }
    activated_ = false;
}

/**
 * Same as the Cubemap::GetActiveSamplerInfo()
 */
bool Texture2d::GetActiveSamplerInfo(std::vector<std::string>& names,
                                     std::vector<GLint>& units) {
    names.clear();
    names.push_back(std::string("samplerObj"));
    units.clear();
    units.push_back(0);
    
    return true;
}

bool Texture2d::Activate(void) {
    glBindTexture(texId_, GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE0 + 0);
    activated_ = true;
    return true;
}

GLuint Texture2d::GetTexType() {
    return GL_TEXTURE_2D;
}

GLuint Texture2d::GetTexId() {
    return texId_;
}

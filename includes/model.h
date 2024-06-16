#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <mesh.h>
#include <shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

unsigned int importTexture(const char *name, const std::string &path) {
    std::string file = path + '/' + std::string(name);
    unsigned int id;
    glGenTextures(1, &id);

    int width, height, components;
    unsigned char *data =
        stbi_load(file.c_str(), &width, &height, &components, 0);
    if (data) {
        GLenum format = components == 1   ? GL_RED
                        : components == 3 ? GL_RGB
                                          : GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    } else {
        std::cerr << "ERROR LOADING TEXTURE AT " << path << std::endl;
        stbi_image_free(data);
    }
    return id;
}

class Model {
   public:
    Model(std::string &path) { load(path); }
    void draw(Shader &shader) {
        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].draw(shader);
        }
    }

   private:
    std::vector<Mesh> meshes;
    std::string path;
    std::vector<Texture> loadedTextures;

    void load(std::string &path) {
        Assimp::Importer importer;
        const aiScene *scene =
            importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
            !scene->mRootNode) {
            std::cerr << "ERROR ASSIMP\n"
                      << importer.GetErrorString() << std::endl;
            return;
        }
        this->path = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }
    void processNode(aiNode *node, const aiScene *scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }
    Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.position =
                glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,
                          mesh->mVertices[i].z);
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                                      mesh->mNormals[i].z);
            if (mesh->mTextureCoords[0]) {
                vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x,
                                             mesh->mTextureCoords[0][i].y);
            } else
                vertex.texCoords = glm::vec2(0.0f);
            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        if (mesh->mMaterialIndex >= 0) {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<Texture> diffuse =
                loadTextures(material, aiTextureType_DIFFUSE, DIFFUSE);
            textures.insert(textures.end(), diffuse.begin(), diffuse.end());
            std::vector<Texture> specular =
                loadTextures(material, aiTextureType_SPECULAR, SPECULAR);
            textures.insert(textures.end(), specular.begin(), specular.end());
        }

        return Mesh(vertices, indices, textures);
    }
    std::vector<Texture> loadTextures(aiMaterial *material, aiTextureType type,
                                      TextureType typeName) {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
            aiString string;
            material->GetTexture(type, i, &string);
            bool skip = false;
            for (unsigned int j = 0; j < loadedTextures.size(); j++) {
                if (loadedTextures[j].path == std::string(string.C_Str())) {
                    textures.push_back(loadedTextures[j]);
                    skip = true;
                    break;
                }
            }
            if (skip) continue;
            Texture texture;
            texture.id = importTexture(string.C_Str(), path);
            texture.type = typeName;
            texture.path = string.C_Str();
            textures.push_back(texture);
            loadedTextures.push_back(texture);
        }
        return textures;
    }
};

#endif
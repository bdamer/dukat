#pragma once

#include "buffers.h"
#include "renderer.h"

namespace dukat
{
    class MeshData;
    class ShaderProgram;
    struct Texture;

    class EffectPass
    { 
    private:
        std::unique_ptr<FrameBuffer> fbo;
        std::unique_ptr<MeshData> mesh;
        ShaderProgram* program;
        // Input textures
        Texture* texture[Renderer::max_texture_units];
        // need list of attributes
        std::unordered_map<std::string,std::vector<GLfloat>> attributes;

    public:
        bool disable_blend; // disables blending during effect rendering

        EffectPass(void);
        ~EffectPass(void) { }

        // Sets frame buffer.
        void set_fbo(std::unique_ptr<FrameBuffer> fbo) { this->fbo = std::move(fbo); }
        // Sets shader program.
        void set_program(ShaderProgram* program) { this->program = program; }
        // Sets texture stage.
        void set_texture(Texture* texture, int index = 0);
        // Sets shader attribute.
        void set_attribute(const std::string& name, const std::vector<GLfloat>& attr) { attributes[name] = attr; }
		// Returns target texture.
		Texture* get_target_buffer(void) const { return (fbo != nullptr ? fbo->texture.get() : nullptr); }
		// Renders effect pass.
        void render(Renderer* renderer);
    };
}
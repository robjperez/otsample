#include "renderer.h"
#include "imgui.h"

#include <iostream>

#include <stdlib.h>

using namespace std;

Renderer::Renderer(const std::string name) : last_frame(nullptr), name(name), image_texture(0) {
    // OpenGL initialization
    // THIS MUST HAPPEN IN THE MAIN THREAD!
    glGenTextures(1, &this->image_texture);
    glBindTexture(GL_TEXTURE_2D, this->image_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::render() {
    if (this->last_frame == nullptr) {
        return;
    }
    ImGui::Begin(this->name.c_str());
    {
        this->mutex.lock();
        const uint8_t* pixels = otc_video_frame_get_plane_binary_data(this->last_frame, static_cast<enum otc_video_frame_plane>(0));
        auto w = otc_video_frame_get_width(this->last_frame);
        auto h = otc_video_frame_get_height(this->last_frame);

        glBindTexture(GL_TEXTURE_2D, this->image_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
        ImGui::Image((void *)(intptr_t)this->image_texture, ImVec2(w, h));
        this->mutex.unlock();
    }
    ImGui::End();
}

void Renderer::set_frame(const otc_video_frame* frame) {
    this->mutex.lock();
    if (this->last_frame != nullptr) {
        otc_video_frame_delete(this->last_frame);
    }
    this->last_frame = otc_video_frame_convert(OTC_VIDEO_FRAME_FORMAT_ARGB32, frame);
    this->mutex.unlock();
}

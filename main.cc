// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <opentok.h>

#include <iostream>
#include <map>
#include <memory>

#include "renderer.h"

using namespace std;
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

const char API_KEY[] = "";
const char TOKEN[] = "";
const char SESSION_ID[] = "";

map<string, unique_ptr<Renderer>> renderer_map;

static otc_session* session = nullptr;
static otc_publisher* publisher = nullptr;
static otc_subscriber* subscriber = nullptr;

static void on_subscriber_error(otc_subscriber* subscriber,
                                void *user_data,
                                const char* error_string,
                                enum otc_subscriber_error_code error) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
  std::cout << "Subscriber error. Error code: " << error_string << std::endl;
}

static void on_session_connected(otc_session *session, void *user_data) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
  otc_session_publish(session, publisher);
}

static void on_session_connection_created(otc_session *session,
                                          void *user_data,
                                          const otc_connection *connection) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
}

static void on_session_connection_dropped(otc_session *session,
                                          void *user_data,
                                          const otc_connection *connection) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
}

static void on_subscriber_render_frame(otc_subscriber *subscriber,
                                       void *user_data,
                                       const otc_video_frame *frame) {
  otc_stream* stream = otc_subscriber_get_stream(subscriber);
  string streamId(otc_stream_get_id(stream));

  renderer_map[streamId]->set_frame(frame);
}

static void on_session_stream_received(otc_session *session,
                                       void *user_data,
                                       const otc_stream *stream) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
  otc_subscriber_callbacks cb = {0};
  cb.on_render_frame = on_subscriber_render_frame;

  string streamId(otc_stream_get_id(stream));
  unique_ptr<Renderer> ptr(new Renderer(streamId));
  renderer_map[streamId] = std::move(ptr);

  subscriber = otc_subscriber_new(stream, &cb);
  otc_session_subscribe(session, subscriber);
}

static void on_session_stream_dropped(otc_session *session,
                                      void *user_data,
                                      const otc_stream *stream) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
}

static void on_session_disconnected(otc_session *session, void *user_data) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
}

static void on_session_error(otc_session *session,
                             void *user_data,
                             const char *error_string,
                             enum otc_session_error_code error) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
  std::cout << "Session error. Error : " << error_string << std::endl;
}
static void on_otc_log_message(const char* message) {
  std::cout <<  __FUNCTION__ << ":" << message << std::endl;
}

static void on_publisher_stream_created(otc_publisher *publisher,
                                        void *user_data,
                                        const otc_stream *stream) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
}


static void on_publisher_render_frame(otc_publisher *publisher,
                                      void *user_data,

                                      const otc_video_frame *frame) {
  if (renderer_map["PUBLISHER"] != nullptr) {
    renderer_map["PUBLISHER"]->set_frame(frame);
  }
}

static void on_publisher_stream_destroyed(otc_publisher *publisher,
                                          void *user_data,
                                          const otc_stream *stream) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
}

static void on_publisher_error(otc_publisher *publisher,
                               void *user_data,
                               const char* error_string,
                               enum otc_publisher_error_code error_code) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
  std::cout << "Publisher error. Error code: " << error_string << std::endl;
}

static void init_ot() {
  if (otc_init(nullptr) != OTC_SUCCESS) {
      std::cout << "Could not init OpenTok library" << std::endl;
      return;
    }
    otc_log_set_logger_callback(on_otc_log_message);
    otc_log_enable(OTC_LOG_LEVEL_INFO);

    struct otc_session_callbacks session_callbacks = {0};
    session_callbacks.on_connected = on_session_connected;
    session_callbacks.on_connection_created = on_session_connection_created;
    session_callbacks.on_connection_dropped = on_session_connection_dropped;
    session_callbacks.on_stream_received = on_session_stream_received;
    session_callbacks.on_stream_dropped = on_session_stream_dropped;
    session_callbacks.on_disconnected = on_session_disconnected;
    session_callbacks.on_error = on_session_error;
    session = otc_session_new(API_KEY, SESSION_ID, &session_callbacks);
    if (session == nullptr) {
      cout << "ERROR creatng session" << endl;
    }
}

void create_publisher() {
    struct otc_publisher_callbacks publisher_callbacks = {0};
    publisher_callbacks.on_stream_created = on_publisher_stream_created;
    publisher_callbacks.on_render_frame = on_publisher_render_frame;
    publisher_callbacks.on_stream_destroyed = on_publisher_stream_destroyed;
    publisher_callbacks.on_error = on_publisher_error;

    unique_ptr<Renderer> ptr(new Renderer("PUBLISHER"));
    renderer_map["PUBLISHER"] = std::move(ptr);

    publisher = otc_publisher_new("name",
                                  nullptr, /* Use WebRTC's video capturer. */
                                  &publisher_callbacks);



    if (publisher == nullptr) {
      std::cout << "Error building publisher" << std::endl;
    }
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "opentok linux sample app", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    init_ot();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
      glfwPollEvents();
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();


//    ImGui::ShowDemoWindow(&show_demo_window);

      ImGui::Begin("Control Panel");
      if (ImGui::Button("Create Publisher")) {
        cout << "Creating Publisher" << endl;
        create_publisher();
      }
      if (ImGui::Button("Publish")) {
        cout << "Publising..." << endl;
        otc_session_publish(session, publisher);
      }
      if (ImGui::Button("Connect")) {
        cout << "Connecting Session" << endl;
        otc_session_connect(session, TOKEN);
      }
      ImGui::End();

      for (auto const& el : renderer_map) {
        el.second->render();
      }

      // Rendering
      ImGui::Render();
      int display_w, display_h;
      glfwGetFramebufferSize(window, &display_w, &display_h);
      glViewport(0, 0, display_w, display_h);
      glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

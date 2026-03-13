#include <cassert>
#include <iostream>
//
#include <glad/glad.h>
#include <windef.h>
//
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

//
#include <Windows.h>
#include <memory>
#include <renderdoc_app.h>

//
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

static void *GetGLContextPtr(GLFWwindow *window) {
  if (!window)
    return nullptr;

  // Windows: 获取当前线程的 HGLRC
  return wglGetCurrentContext();

  return nullptr;
}

static HWND g_hwnd = nullptr;

static void DebugBox(const char *message) {
  MessageBoxA(g_hwnd, message, "Debug", MB_OK | MB_ICONINFORMATION);
}

class RenderdocHelper {
public:
  static bool Init() {
    loader = std::make_unique<Loader>();
    return loader->IsLoaded();
  }

  static void StartCapture() {
    if (loader && loader->IsLoaded())
      loader->GetAPI()->StartFrameCapture(NULL, NULL);
  }

  static void EndCapture() {
    if (loader && loader->IsLoaded())
      loader->GetAPI()->EndFrameCapture(NULL, NULL);
  }

  static bool TriggerCapture() {
    if (loader && loader->IsLoaded()) {
      loader->GetAPI()->TriggerCapture();
      return true;
    }

    return false;
  }

  static bool TriggerMultiFrameCapture(uint32_t numFrames) {
    if (loader && loader->IsLoaded()) {
      loader->GetAPI()->TriggerMultiFrameCapture(numFrames);
      return true;
    }

    return false;
  }

  static void SetCaptureTitle(const std::string &title) {
    if (loader && loader->IsLoaded())
      loader->GetAPI()->SetCaptureTitle(title.c_str());
  }

  static void SetActiveWindow(void *device, void *wndHandle) {
    if (loader && loader->IsLoaded())
      loader->GetAPI()->SetActiveWindow(device, wndHandle);
  }

  static void SetActiveWindow(GLFWwindow *window) {
    if (loader && loader->IsLoaded()) {
      void *device = GetGLContextPtr(window);
      void *wndHandle = glfwGetWin32Window(window);

      if (!device) {
        DebugBox("[RenderDoc] Failed to get GL context");

        return;
      }

      loader->GetAPI()->SetActiveWindow(device, wndHandle);
    }
  }

private:
  class Loader {
  public:
    Loader() {
      HMODULE mod = LoadLibraryA("C:\\Program Files\\RenderDoc\\renderdoc.dll");
      if (mod) {
        pfn_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        if (pfn_GetAPI) {
          pfn_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&_api);
        }
      }
    }

    bool IsLoaded() const { return _api != nullptr; }
    RENDERDOC_API_1_1_2 *GetAPI() { return _api; }

  private:
    RENDERDOC_API_1_1_2 *_api;
    pRENDERDOC_GetAPI pfn_GetAPI;
  };

  static std::unique_ptr<Loader> loader;
};

std::unique_ptr<RenderdocHelper::Loader> RenderdocHelper::loader = nullptr;
static bool g_CaptureTrigger = false;

int main() {
  // 在创建窗口之前初始化RenderDoc，以确保在窗口创建时能够正确设置捕获窗口
  RenderdocHelper::Init();

  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello Triangle", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  g_hwnd = glfwGetWin32Window(window);

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetKeyCallback(window, key_callback);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // 在加载GL之后，调用GL命令之前，设置RenderDoc的捕获窗口
  RenderdocHelper::SetActiveWindow(window);
  // build and compile our shader program
  // ------------------------------------
  // vertex shader
  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  // check for shader compile errors
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
  // fragment shader
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  // check for shader compile errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
  // link shaders
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  // check for linking errors
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  float vertices[] = {
      -0.5f, -0.5f, 0.0f, // left
      0.5f,  -0.5f, 0.0f, // right
      0.0f,  0.5f,  0.0f  // top
  };

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
  // then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO
  // as the vertex attribute's bound vertex buffer object so afterwards we can
  // safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally
  // modify this VAO, but this rarely happens. Modifying other VAOs requires a
  // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
  // VBOs) when it's not directly necessary.
  glBindVertexArray(0);

  // uncomment this call to draw in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // input
    // -----
    processInput(window);

    if (g_CaptureTrigger) {
      RenderdocHelper::StartCapture();
      RenderdocHelper::SetCaptureTitle("Hello Triangle Capture");
    }

    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw our first triangle
    glUseProgram(shaderProgram);
    glBindVertexArray(
        VAO); // seeing as we only have a single VAO there's no need to bind it
              // every time, but we'll do so to keep things a bit more organized
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // glBindVertexArray(0); // no need to unbind it every time

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    // -------------------------------------------------------------------------------

    if (g_CaptureTrigger) {
      RenderdocHelper::EndCapture();
      g_CaptureTrigger = false;
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // optional: de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderProgram);

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    g_CaptureTrigger = true;
    // RenderdocHelper::TriggerCapture();
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

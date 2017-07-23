#include <GL/glew.h>
#include "imgui/imgui.h"
#include "imgui.hh"

static float  g_Time = 0.0f;
static bool   g_MousePressed[3] = { false, false, false };
static float  g_MouseWheel = 0.0f;
static GLuint g_FontTexture = 0;
static int    g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int    g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int    g_AttribLocationPosition = 0, g_AttribLocationUV = 0
    , g_AttribLocationColor = 0;
static unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;

static void imgui_render_draw_lists(ImDrawData* draw_data) {
  ImGuiIO& io = ImGui::GetIO();
  int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
  int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
  if (fb_width == 0 || fb_height == 0)
    return;
  draw_data->ScaleClipRects(io.DisplayFramebufferScale);

  // Backup GL state
  GLint last_active_texture, last_program, last_texture, last_array_buffer
    , last_element_array_buffer, last_vertex_array, last_blend_src_rgb
    , last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha
    , last_blend_equation_rgb, last_blend_equation_alpha, last_viewport[4]
    , last_scissor_box[4];
  glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
  glActiveTexture(GL_TEXTURE0);
  glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
  glGetIntegerv(GL_BLEND_SRC_RGB, &last_blend_src_rgb);
  glGetIntegerv(GL_BLEND_DST_RGB, &last_blend_dst_rgb);
  glGetIntegerv(GL_BLEND_SRC_ALPHA, &last_blend_src_alpha);
  glGetIntegerv(GL_BLEND_DST_ALPHA, &last_blend_dst_alpha);
  glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
  glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
  glGetIntegerv(GL_VIEWPORT, last_viewport);
  glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
  GLboolean last_enable_blend = glIsEnabled(GL_BLEND)
    , last_enable_cull_face = glIsEnabled(GL_CULL_FACE)
    , last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST)
    , last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);

  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
  const float ortho_projection[4][4] = {
    { 2.f / io.DisplaySize.x, 0.f,                     0.f, 0.f },
    { 0.f,                    2.f / -io.DisplaySize.y, 0.f, 0.f },
    { 0.f,                    0.f,                    -1.f, 0.f },
    {-1.f,                    1.f,                     0.f, 1.f },
  };
  glUseProgram(g_ShaderHandle);
  glUniform1i(g_AttribLocationTex, 0);
  glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE
      , &ortho_projection[0][0]);
  glBindVertexArray(g_VaoHandle);

  for (int n = 0; n < draw_data->CmdListsCount; ++n) {
    const ImDrawList *cmd_list = draw_data->CmdLists[n];
    const ImDrawIdx *idx_buffer_offset = 0;
    glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
    glBufferData(GL_ARRAY_BUFFER
        , (GLsizeiptr)(cmd_list->VtxBuffer.Size * sizeof(ImDrawVert))
        , (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER
        , (GLsizeiptr)(cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx))
        , (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback)
        pcmd->UserCallback(cmd_list, pcmd);
      else {
        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
        glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w)
            , (int)(pcmd->ClipRect.z - pcmd->ClipRect.x)
            , (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
        glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount
            , sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT
            , idx_buffer_offset);
      }
      idx_buffer_offset += pcmd->ElemCount;
    }
  }

  // Restore modified GL state
  glUseProgram(last_program);
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glActiveTexture(last_active_texture);
  glBindVertexArray(last_vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
  glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
  glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb
      , last_blend_src_alpha, last_blend_dst_alpha);
  last_enable_blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
  last_enable_cull_face ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
  last_enable_depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
  last_enable_scissor_test ? glEnable(GL_SCISSOR_TEST)
    : glDisable(GL_SCISSOR_TEST);
  glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2]
      , (GLsizei)last_viewport[3]);
  glScissor(last_scissor_box[0], last_scissor_box[1]
      , (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

static const char* imgui_get_clipboard_text(void*) {
  return SDL_GetClipboardText();
}

static void imgui_set_clipboard_text(void*, const char* text) {
  SDL_SetClipboardText(text);
}

static bool imgui_create_ogl_state() {
  const GLchar *vertex_shader =
    "#version 330\n"
    "uniform mat4 ProjMtx;\n"
    "in vec2 Position;\n"
    "in vec2 UV;\n"
    "in vec4 Color;\n"
    "out vec2 Frag_UV;\n"
    "out vec4 Frag_Color;\n"
    "void main()\n"
    "{\n"
    "	Frag_UV = UV;\n"
    "	Frag_Color = Color;\n"
    "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
    "}\n";
  const GLchar* fragment_shader =
    "#version 330\n"
    "uniform sampler2D Texture;\n"
    "in vec2 Frag_UV;\n"
    "in vec4 Frag_Color;\n"
    "out vec4 Out_Color;\n"
    "void main()\n"
    "{\n"
    "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
    "}\n";
  g_ShaderHandle = glCreateProgram();
  g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
  g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
  glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
  glCompileShader(g_VertHandle);
  glCompileShader(g_FragHandle);
  glAttachShader(g_ShaderHandle, g_VertHandle);
  glAttachShader(g_ShaderHandle, g_FragHandle);
  glLinkProgram(g_ShaderHandle);

  g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
  g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
  g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
  g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
  g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

  glGenBuffers(1, &g_VboHandle);
  glGenBuffers(1, &g_ElementsHandle);

  glGenVertexArrays(1, &g_VaoHandle);
  glBindVertexArray(g_VaoHandle);
  glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
  glEnableVertexAttribArray(g_AttribLocationPosition);
  glEnableVertexAttribArray(g_AttribLocationUV);
  glEnableVertexAttribArray(g_AttribLocationColor);

  glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE
      , sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos));
  glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE
      , sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv));
  glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE
      , sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col));

  ImGuiIO& io = ImGui::GetIO();
  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  glGenTextures(1, &g_FontTexture);
  glBindTexture(GL_TEXTURE_2D, g_FontTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA
      , GL_UNSIGNED_BYTE, pixels);
  io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

  return true;
}

bool imgui_init(SDL_Window* window) {
  ImGuiIO& io = ImGui::GetIO();
  io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
  io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
  io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
  io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
  io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
  io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
  io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
  io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
  io.KeyMap[ImGuiKey_A] = SDLK_a;
  io.KeyMap[ImGuiKey_C] = SDLK_c;
  io.KeyMap[ImGuiKey_V] = SDLK_v;
  io.KeyMap[ImGuiKey_X] = SDLK_x;
  io.KeyMap[ImGuiKey_Y] = SDLK_y;
  io.KeyMap[ImGuiKey_Z] = SDLK_z;

  io.RenderDrawListsFn = imgui_render_draw_lists;
  io.SetClipboardTextFn = imgui_set_clipboard_text;
  io.GetClipboardTextFn = imgui_get_clipboard_text;
  io.ClipboardUserData = NULL;

  io.IniFilename = NULL;

  io.Fonts->AddFontFromFileTTF("gudea.ttf", 19.0f);

#ifdef _WIN32
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(window, &wmInfo);
  io.ImeWindowHandle = wmInfo.info.win.window;
#else
  (void)window;
#endif

  return true;
}

void imgui_destroy() {
  if (g_VaoHandle)
    glDeleteVertexArrays(1, &g_VaoHandle);
  if (g_VboHandle)
    glDeleteBuffers(1, &g_VboHandle);
  if (g_ElementsHandle)
    glDeleteBuffers(1, &g_ElementsHandle);
  g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

  if (g_ShaderHandle && g_VertHandle)
    glDetachShader(g_ShaderHandle, g_VertHandle);
  if (g_VertHandle)
    glDeleteShader(g_VertHandle);
  g_VertHandle = 0;

  if (g_ShaderHandle && g_FragHandle)
    glDetachShader(g_ShaderHandle, g_FragHandle);
  if (g_FragHandle)
    glDeleteShader(g_FragHandle);
  g_FragHandle = 0;

  if (g_ShaderHandle)
    glDeleteProgram(g_ShaderHandle);
  g_ShaderHandle = 0;

  if (g_FontTexture) {
    glDeleteTextures(1, &g_FontTexture);
    ImGui::GetIO().Fonts->TexID = 0;
    g_FontTexture = 0;
  }
  ImGui::Shutdown();
}

void imgui_new_frame(SDL_Window* window) {
  if (!g_FontTexture)
    imgui_create_ogl_state();

  ImGuiIO& io = ImGui::GetIO();

  int w, h;
  int display_w, display_h;
  SDL_GetWindowSize(window, &w, &h);
  SDL_GL_GetDrawableSize(window, &display_w, &display_h);
  io.DisplaySize = ImVec2((float)w, (float)h);
  io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0
      , h > 0 ? ((float)display_h / h) : 0);

  float current_time = (float)SDL_GetTicks() / 1000.f;
  io.DeltaTime = g_Time > 0.f ? current_time - g_Time : 1.f / 60.f;
  g_Time = current_time;

  int mx, my;
  Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
  if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
    io.MousePos = ImVec2((float)mx, (float)my);
  else
    io.MousePos = ImVec2(-1, -1);

  io.MouseDown[0] = g_MousePressed[0]
    || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
  io.MouseDown[1] = g_MousePressed[1]
    || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
  io.MouseDown[2] = g_MousePressed[2]
    || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
  g_MousePressed[0] = g_MousePressed[1] = g_MousePressed[2] = false;

  io.MouseWheel = g_MouseWheel;
  g_MouseWheel = 0.0f;

  SDL_ShowCursor(io.MouseDrawCursor ? 0 : 1);

  ImGui::NewFrame();
}

bool imgui_process_event(SDL_Event* event) {
  ImGuiIO& io = ImGui::GetIO();
  switch (event->type) {
    case SDL_MOUSEWHEEL: {
      if (event->wheel.y > 0)
        g_MouseWheel = 1;
      if (event->wheel.y < 0)
        g_MouseWheel = -1;
      return true;
    }
    case SDL_MOUSEBUTTONDOWN: {
      if (event->button.button == SDL_BUTTON_LEFT)   g_MousePressed[0] = true;
      if (event->button.button == SDL_BUTTON_RIGHT)  g_MousePressed[1] = true;
      if (event->button.button == SDL_BUTTON_MIDDLE) g_MousePressed[2] = true;
      return true;
    }
    case SDL_TEXTINPUT: {
      io.AddInputCharactersUTF8(event->text.text);
      return true;
    }
    case SDL_KEYDOWN:
    case SDL_KEYUP: {
      int key = event->key.keysym.sym & ~SDLK_SCANCODE_MASK;
      io.KeysDown[key] = (event->type == SDL_KEYDOWN);
      io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
      io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
      io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
      io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
      return true;
    }
  }
  return false;
}


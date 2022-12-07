#include <stdio.h>
#include <windows.h>
#include "dependencies/glad/glad.h"

#ifdef __cplusplus
extern "C" {
#endif
__declspec(dllexport) int NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#ifdef __cplusplus
}
#endif

PFNGLCREATEBUFFERSPROC glCreateBuffers;
PFNGLNAMEDBUFFERSTORAGEPROC glNamedBufferStorage;
PFNGLNAMEDBUFFERSUBDATAPROC glNamedBufferSubData;
PFNGLBINDBUFFERBASEPROC glBindBufferBase;

PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;

PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

PFNGLCREATETEXTURESPROC glCreateTextures;
PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D;
PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;

PFNGLDRAWARRAYSPROC glDrawArrays;

void *glGetFunc(const char *name)
{
    void *p = (void *) wglGetProcAddress(name);
    if (p == NULL || (p == (void *) 0x1) || (p == (void *) 0x2) || (p == (void *) 0x3) || (p == (void *) -1))
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void *) GetProcAddress(module, name);
    }
    return p;
}

void glInitFunc() {
    glCreateBuffers      = (PFNGLCREATEBUFFERSPROC) glGetFunc("glCreateBuffers");
    glNamedBufferStorage = (PFNGLNAMEDBUFFERSTORAGEPROC) glGetFunc("glNamedBufferStorage");
    glNamedBufferSubData = (PFNGLNAMEDBUFFERSUBDATAPROC) glGetFunc("glNamedBufferSubData");
    glBindBufferBase     = (PFNGLBINDBUFFERBASEPROC) glGetFunc("glBindBufferBase");

    glCreateProgram      = (PFNGLCREATEPROGRAMPROC) glGetFunc("glCreateProgram");
    glAttachShader       = (PFNGLATTACHSHADERPROC) glGetFunc("glAttachShader");
    glLinkProgram        = (PFNGLLINKPROGRAMPROC) glGetFunc("glLinkProgram");
    glUseProgram         = (PFNGLUSEPROGRAMPROC) glGetFunc("glUseProgram");

    glDispatchCompute    = (PFNGLDISPATCHCOMPUTEPROC) glGetFunc("glDispatchCompute");

    glCreateShader       = (PFNGLCREATESHADERPROC) glGetFunc("glCreateShader");
    glShaderSource       = (PFNGLSHADERSOURCEPROC) glGetFunc("glShaderSource");
    glCompileShader      = (PFNGLCOMPILESHADERPROC) glGetFunc("glCompileShader");
    glGetShaderiv        = (PFNGLGETSHADERIVPROC) glGetFunc("glGetShaderiv");
    glGetShaderInfoLog   = (PFNGLGETSHADERINFOLOGPROC) glGetFunc("glGetShaderInfoLog");

    glCreateTextures     = (PFNGLCREATETEXTURESPROC) glGetFunc("glCreateTextures");
    glTextureStorage2D   = (PFNGLTEXTURESTORAGE2DPROC) glGetFunc("glTextureStorage2D");
    glBindImageTexture   = (PFNGLBINDIMAGETEXTUREPROC) glGetFunc("glBindImageTexture");

    glDrawArrays         = (PFNGLDRAWARRAYSPROC) glGetFunc("glDrawArrays");
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void PrintLastError(const char *name) {
    LPSTR msg;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &msg, 0, NULL);
    MessageBox(NULL, msg, name, MB_OK | MB_ICONERROR);
    LocalFree(msg);
    exit(1);
}

GLuint CompileGLShaderFile(const char *filename, GLenum shaderType) {
    HANDLE shaderFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ,
                                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (shaderFile == INVALID_HANDLE_VALUE) {
        PrintLastError(filename);
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(shaderFile, &size)) {
        PrintLastError(filename);
    }

    char *shaderSource = malloc(size.QuadPart + 1);
    DWORD readSize;
    if (!ReadFile(shaderFile, (LPVOID) shaderSource, size.QuadPart, &readSize, NULL) || readSize != size.QuadPart) {
        PrintLastError(filename);
    }
    // Put 0 char on end of the source buffer for indicating this file ended.
    shaderSource[size.QuadPart] = '\0';
    CloseHandle(shaderFile);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const GLchar *const *) &shaderSource, 0);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = (GLchar *) malloc(length);
        glGetShaderInfoLog(shader, length, &length, info);
        MessageBox(NULL, info, "Shader Error", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    free(shaderSource);
    return shader;
}

int main(int argc, char **argv)
{
    int width = argc > 1 ? atoi(argv[1]) : 1600;
    int height = width;

    SetProcessDPIAware();

    WNDCLASS windowClass = {0};
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "OpenGL Ray Tracing";
    windowClass.lpfnWndProc = WindowProcedure;
    RegisterClass(&windowClass);

    HWND wnd = CreateWindow(windowClass.lpszClassName, windowClass.lpszClassName, WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, width + 2, height + 25, NULL, NULL, NULL, NULL);
    HDC dc = GetDC(wnd);

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

    int pfdId = ChoosePixelFormat(dc, &pfd);
    SetPixelFormat(dc, pfdId, &pfd);

    HGLRC rc = wglCreateContext(dc);
    wglMakeCurrent(dc, rc);
    ShowWindow(wnd, SW_NORMAL);

    // Start of OpenGL Program
    glInitFunc();

    unsigned int finalProgram;
    {
        unsigned int vertexShader = CompileGLShaderFile("screenQuad.glsl", GL_VERTEX_SHADER);
        unsigned int fragmentShader = CompileGLShaderFile("final.glsl", GL_FRAGMENT_SHADER);

        finalProgram = glCreateProgram();
        glAttachShader(finalProgram, vertexShader);
        glAttachShader(finalProgram, fragmentShader);
        glLinkProgram(finalProgram);
    }

    unsigned int computeProgram;
    {
        unsigned int computeShader = CompileGLShaderFile("rayTracer.glsl", GL_COMPUTE_SHADER);

        computeProgram = glCreateProgram();
        glAttachShader(computeProgram, computeShader);
        glLinkProgram(computeProgram);
    }

    unsigned int computeResult;
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &computeResult);
        glTextureStorage2D(computeResult, 1, GL_RGBA32F, width, height);
    }

    unsigned int basicDataUBO;
    {
        glCreateBuffers(1, &basicDataUBO);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, basicDataUBO);
        float data[] = 
        {
            // InvProjection
            1.2571722f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.2571722f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -99.999504f, 100.0005f,

            // InvView
            -0.11667083f, -0.060631607f, -0.9913181f, -18.930002f,
            0.0f, 0.99813473f, -0.061048526f, -5.07f,
            0.9931706f, -0.0071225823f, -0.1164532f, -17.75f,
            0.0f, 0.0f, 0.0f, 1.0f,

            // ViewPos
            -18.93f, -5.07f, -17.75f,

            // Rendered Frame
            0.0f
        };
        glNamedBufferStorage(basicDataUBO, sizeof(data), data, GL_DYNAMIC_STORAGE_BIT);
    }

    int renderedFrame = 0;
    unsigned long long start = GetTickCount64();
    MSG msg = {0};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            glUseProgram(computeProgram);
            glBindImageTexture(0u, computeResult, 0, FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
            glNamedBufferSubData(basicDataUBO, sizeof(float) * 4 * 4 * 2 + sizeof(float) * 3, sizeof(int), &renderedFrame);
            renderedFrame++;
            glDispatchCompute((width * height + 32 - 1) / 32, 1, 1);

            glBindImageTexture(0u, computeResult, 0, FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glUseProgram(finalProgram);
            glDrawArrays(GL_QUADS, 0, 4);
            char title[64];
            sprintf(title, "OpenGL Ray Tracing - FPS: %.2f", renderedFrame * 1000 / (double) (GetTickCount64() - start));
            SetWindowText(wnd, title);
            SwapBuffers(dc);
        }
    }
    return 0;
}

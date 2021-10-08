namespace Havana::Graphics::OpenGL::OGL
{
    using glXCreateContextAttribsARBProc = 
		GLXContext (*)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

    #define GL_FUNCTIONS(X) \
        X(PFNGLENABLEPROC,                      glEnable                    )\
        X(PFNGLDISABLEPROC,                     glDisable                   )\
        X(PFNGLBLENFUNCPROC,                    glBlendFunc                 )\
        X(PFNGLVIEWPORTPROC,                    glViewport                  )\
        X(PFNGLCLEARCOLORPROC,                  glClearColor                )\
        X(PFNGLCLEARPROC,                       glClear                     )\
        X(PFNGLDRAWARRAYSPROC,                  glDrawArrays                )\
        X(PFNGLCREATEBUFFERSPROC,               glCreateBuffers             )\
        X(PFNGLNAMEDBUFFERSTORAGEPROC,          glNamedBufferStorage        )\
        X(PFNGLBINDVERTEXARRAYPROC,             glBindVertexArray           )\
        X(PFNGLCREATEVERTEXARRAYSPROC,          glCreateVertexArrays        )\
        X(PFNGLVERTEXARRAYATTRIBBINDINGPROC,    glVertexArrayAttribBinding  )\
        X(PFNGLVERTEXARRAYVERTEXBUFFERPROC,     glVertexArrayVertexBuffer   )\
        X(PFNGLVERTEXARRAYATTRIBFORMATPROC,     glVertexArrayAttribFormat   )\
        X(PFNGLENABLEVERTEXARRAYATTRIBPROC,     glEnableVertexArrayAttrib   )\
        X(PFNGLCREATESHADERPROGRAMVPROC,        glCreateShaderProgramv      )\
        X(PFNGLGETPROGRAMIVPROC,                glGetProgramiv              )\
        X(PFNGLGETPROGRAMINFOLOGPROC,           glGetProgramInfoLog         )\
        X(PFNGLGENPROGRAMPIPELINESPROC,         glGenProgramPipelines       )\
        X(PFNGLUSEPROGRAMSTAGESPROC,            glUseProgramStages          )\
        X(PFNGLBINDPROGRAMPIPELINEPROC,         glBindProgramPipeline       )\
        X(PFNGLPROGRAMUNIFORMMATRIX2FVPROC,     glProgramUniformMatrix2fv   )\
        X(PFNGLBINDTEXTUREUNITPROC,             glBindTextureUnit           )\
        X(PFNGLCREATETEXTURESPROC,              glCreateTextures            )\
        X(PFNGLTEXTUREPARAMETERIPROC,           glTextureParameteri         )\
        X(PFNGLTEXTURESTORAGE2DPROC,            glTextureStorage2D          )\
        X(PFNGLTEXTURESUBIMAGE2DPROC,           glTextureSubImage2D         )\
        X(PFNGLDEBUGMESSAGECALLBACKPROC,        glDebugMessageCallback      )

    #define X(type, name) static type name;
    GL_FUNCTIONS(X)
    #undef X
}
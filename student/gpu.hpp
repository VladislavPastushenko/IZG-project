/*!
 * @file
 * @brief This file contains class that represents graphic card.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */
#pragma once

#include <student/fwd.hpp>
#include <list>
#include <vector>


 /**
  * @brief This struct represents GPUs buffer.
  */
struct Buffer {
    BufferID id;///< buffer id
    void* data;///< buffer data
};

struct Indexing {
    BufferID buf;
    IndexType type;
    bool enabled = false;
};

struct HeadSettings {
    uint64_t offset;
    uint64_t stride;
    AttributeType type;
    bool enabled = false;
    BufferID buf;
};

struct VertexPullerSettings {
    Indexing indexing;
    HeadSettings* heads;
    VertexPullerID id;
};

struct Attributes {
    AttributeType type;
    uint32_t id;
};

struct Program {
    VertexShader VS;
    FragmentShader FS;
    Uniforms un;
    ProgramID id;
    AttributeType attributes[maxAttributes];
};





/**
 * @brief This class represent software GPU
 */
class GPU{
  public:
    GPU();
    virtual ~GPU();

    //buffer object commands
    BufferID  createBuffer           (uint64_t size);
    void      deleteBuffer           (BufferID buffer);
    void      setBufferData          (BufferID buffer,uint64_t offset,uint64_t size,void const* data);
    void      getBufferData          (BufferID buffer,uint64_t offset,uint64_t size,void      * data);
    bool      isBuffer               (BufferID buffer);

    //vertex array object commands (vertex puller)
    ObjectID  createVertexPuller     ();
    void      deleteVertexPuller     (VertexPullerID vao);
    void      setVertexPullerHead    (VertexPullerID vao,uint32_t head,AttributeType type,uint64_t stride,uint64_t offset,BufferID buffer);
    void      setVertexPullerIndexing(VertexPullerID vao,IndexType type,BufferID buffer);
    void      enableVertexPullerHead (VertexPullerID vao,uint32_t head);
    void      disableVertexPullerHead(VertexPullerID vao,uint32_t head);
    void      bindVertexPuller       (VertexPullerID vao);
    void      unbindVertexPuller     ();
    bool      isVertexPuller         (VertexPullerID vao);

    //program object commands
    ProgramID createProgram          ();
    void      deleteProgram          (ProgramID prg);
    void      attachShaders          (ProgramID prg,VertexShader vs,FragmentShader fs);
    void      setVS2FSType           (ProgramID prg,uint32_t attrib,AttributeType type);
    void      useProgram             (ProgramID prg);
    bool      isProgram              (ProgramID prg);
    void      programUniform1f       (ProgramID prg,uint32_t uniformId,float     const&d);
    void      programUniform2f       (ProgramID prg,uint32_t uniformId,glm::vec2 const&d);
    void      programUniform3f       (ProgramID prg,uint32_t uniformId,glm::vec3 const&d);
    void      programUniform4f       (ProgramID prg,uint32_t uniformId,glm::vec4 const&d);
    void      programUniformMatrix4f (ProgramID prg,uint32_t uniformId,glm::mat4 const&d);

    //framebuffer functions
    void      createFramebuffer      (uint32_t width,uint32_t height);
    void      deleteFramebuffer      ();
    void      resizeFramebuffer      (uint32_t width,uint32_t height);
    uint8_t*  getFramebufferColor    ();
    float*    getFramebufferDepth    ();
    uint32_t  getFramebufferWidth    ();
    uint32_t  getFramebufferHeight   ();

    //execution commands
    void      clear                  (float r,float g,float b,float a);
    void      drawTriangles          (uint32_t  nofVertices);

    //user functions 
    void      trianglesClipping      (OutVertex& a, OutVertex& b, OutVertex& c);
    OutVertex Interpolation          (OutVertex& a, OutVertex& b, float f);
    void      Drawing                (OutVertex& a, OutVertex& b, OutVertex& c);
    void      putPixel               (InFragment inFragment);
    void      swapVertex             (OutVertex& a, OutVertex& b);
    void      swapFloat              (float& a, float& b);
    void      postProcesses          (OutVertex& a, OutVertex& b, OutVertex& c);
    void      Line                   (OutVertex a, OutVertex b, glm::vec2* Lines, OutVertex& v0, OutVertex& v1, OutVertex& v2);
    float     getTraingleTop         (OutVertex& a, OutVertex& b, OutVertex& c);
    float     getTraingleBottom      (OutVertex& a, OutVertex& b, OutVertex& c);

    /// \addtogroup gpu_init 00. proměnné, inicializace / deinicializace grafické karty
    /// @{

    std::list<Buffer> BufferList;
    std::list<VertexPullerSettings> VertexPullerList;
    VertexPullerID bindedVPid = emptyID;
    ProgramID ActiveProgramID = emptyID;
    std::list<Program> ProgramList;
    
    uint32_t Width = 0;
    uint32_t Height = 0;

    uint8_t* colorBuf = nullptr;
    float* depthBuf = nullptr;
    /// \todo zde si můžete vytvořit proměnné grafické karty (buffery, programy, ...)
    /// @}
};



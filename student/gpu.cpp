/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */
#include <iostream>
#include <stdexcept>
#include <student/gpu.hpp>
#include <vector>


/// \addtogroup gpu_init
/// @{

/**
 * @brief Constructor of GPU
 */
GPU::GPU(){

  /// \todo Zde můžete alokovat/inicializovat potřebné proměnné grafické karty
}

/**
 * @brief Destructor of GPU
 */
GPU::~GPU(){
    
    unbindVertexPuller();
    
    for (std::list<Buffer>::iterator item = BufferList.begin(); item != BufferList.end(); item++) {
        if (item->data)
            free(item->data);
    }

    
    for (std::list<VertexPullerSettings>::iterator item = VertexPullerList.begin(); item != VertexPullerList.end(); item++) {
        if (item->heads)
            free(item->heads);
        }
        
    deleteFramebuffer();

  /// \todo Zde můžete dealokovat/deinicializovat grafickou kartu
}

/// @}

/** \addtogroup buffer_tasks 01. Implementace obslužných funkcí pro buffery
 * @{
 */

/**
 * @brief This function allocates buffer on GPU.
 *
 * @param size size in bytes of new buffer on GPU.
 *
 * @return unique identificator of the buffer
 */
BufferID GPU::createBuffer(uint64_t size) { 
  /// \todo Tato funkce by měla na grafické kartě vytvořit buffer dat.<br>
  /// Velikost bufferu je v parameteru size (v bajtech).<br>
  /// Funkce by měla vrátit unikátní identifikátor identifikátor bufferu.<br>
  /// Na grafické kartě by mělo být možné alkovat libovolné množství bufferů o libovolné velikosti.<br>
    Buffer newBuffer;
    if (!BufferList.empty())
        newBuffer.id = BufferList.back().id + 1;
    else
        newBuffer.id = 1;

    newBuffer.data = malloc(sizeof(char) * size);
    if (newBuffer.data != NULL) {
        BufferList.push_back(newBuffer);
        return newBuffer.id;
    }

  return emptyID; 
}

/**
 * @brief This function frees allocated buffer on GPU.
 *
 * @param buffer buffer identificator
 */
void GPU::deleteBuffer(BufferID buffer) {
  /// \todo Tato funkce uvolní buffer na grafické kartě.
  /// Buffer pro smazání je vybrán identifikátorem v parameteru "buffer".
  /// Po uvolnění bufferu je identifikátor volný a může být znovu použit při vytvoření nového bufferu.
    std::list<Buffer>::iterator item;
    for (item = BufferList.begin(); item != BufferList.end(); item++) {
        if (item->id == buffer) {
            if (item->data)
                free(item->data);

            BufferList.erase(item);
            break;
        }
    }
}

/**
 * @brief This function uploads data to selected buffer on the GPU
 *
 * @param buffer buffer identificator
 * @param offset specifies the offset into the buffer's data
 * @param size specifies the size of buffer that will be uploaded
 * @param data specifies a pointer to new data
 */
void GPU::setBufferData(BufferID buffer, uint64_t offset, uint64_t size, void const* data) {
  /// \todo Tato funkce nakopíruje data z cpu na "gpu".<br>
  /// Data by měla být nakopírována do bufferu vybraného parametrem "buffer".<br>
  /// Parametr size určuje, kolik dat (v bajtech) se překopíruje.<br>
  /// Parametr offset určuje místo v bufferu (posun v bajtech) kam se data nakopírují.<br>
  /// Parametr data obsahuje ukazatel na data na cpu pro kopírování.<br>
    std::list<Buffer>::iterator item;
    for (item = BufferList.begin(); item != BufferList.end(); item++) {
        if (item->id == buffer) {
                
                memcpy((uint8_t*)item->data+offset, data, size);
            break;
        }
    }
    return;

}

/**
 * @brief This function downloads data from GPU.
 *
 * @param buffer specfies buffer
 * @param offset specifies the offset into the buffer from which data will be returned, measured in bytes. 
 * @param size specifies data size that will be copied
 * @param data specifies a pointer to the location where buffer data is returned. 
 */
void GPU::getBufferData(BufferID buffer,
                        uint64_t offset,
                        uint64_t size,
                        void*    data)
{
  /// \todo Tato funkce vykopíruje data z "gpu" na cpu.
  /// Data by měla být vykopírována z bufferu vybraného parametrem "buffer".<br>
  /// Parametr size určuje kolik dat (v bajtech) se překopíruje.<br>
  /// Parametr offset určuje místo v bufferu (posun v bajtech) odkud se začne kopírovat.<br>
  /// Parametr data obsahuje ukazatel, kam se data nakopírují.<br>
    std::list<Buffer>::iterator item;
    for (item = BufferList.begin(); item != BufferList.end(); item++) {
        if (item->id == buffer) {
            if (item->data != NULL)
                memcpy(data, (uint8_t*)item->data + offset, size);
            break;
        }
    }
    return;
}

/**
 * @brief This function tests if buffer exists
 *
 * @param buffer selected buffer id
 *
 * @return true if buffer points to existing buffer on the GPU.
 */
bool GPU::isBuffer(BufferID buffer) { 
  /// \todo Tato funkce by měla vrátit true pokud buffer je identifikátor existující bufferu.<br>
  /// Tato funkce by měla vrátit false, pokud buffer není identifikátor existujícího bufferu. (nebo bufferu, který byl smazán).<br>
  /// Pro emptyId vrací false.<br>

    std::list<Buffer>::iterator item;
    for (item = BufferList.begin(); item != BufferList.end(); item++) {
        if (item->id == buffer) {
            return true;
        }
    }
  return false; 
}

/// @}

/**
 * \addtogroup vertexpuller_tasks 02. Implementace obslužných funkcí pro vertex puller
 * @{
 */

/**
 * @brief This function creates new vertex puller settings on the GPU,
 *
 * @return unique vertex puller identificator
 */
ObjectID GPU::createVertexPuller     (){
  /// \todo Tato funkce vytvoří novou práznou tabulku s nastavením pro vertex puller.<br>
  /// Funkce by měla vrátit identifikátor nové tabulky.
  /// Prázdná tabulka s nastavením neobsahuje indexování a všechny čtecí hlavy jsou vypnuté.
    VertexPullerSettings newVertexPuller;
    if (!VertexPullerList.empty())
        newVertexPuller.id = VertexPullerList.back().id + 1;
    else
        newVertexPuller.id = 1;

    newVertexPuller.heads = (HeadSettings*)malloc(sizeof(HeadSettings) * maxAttributes);
    VertexPullerList.push_back(newVertexPuller);
    return newVertexPuller.id;
  return emptyID;
}

/**
 * @brief This function deletes vertex puller settings
 *
 * @param vao vertex puller identificator
 */
void     GPU::deleteVertexPuller     (VertexPullerID vao){
  /// \todo Tato funkce by měla odstranit tabulku s nastavení pro vertex puller.<br>
  /// Parameter "vao" obsahuje identifikátor tabulky s nastavením.<br>
  /// Po uvolnění nastavení je identifiktátor volný a může být znovu použit.<br
    std::list<VertexPullerSettings>::iterator item;
    for (item = VertexPullerList.begin(); item != VertexPullerList.end(); item++) {
        if (item->id == vao) {
            if(item->heads)
                free(item->heads);
            VertexPullerList.erase(item);
            break;
        }
    }
}

/**
 * @brief This function sets one vertex puller reading head.
 *
 * @param vao identificator of vertex puller
 * @param head id of vertex puller head
 * @param type type of attribute
 * @param stride stride in bytes
 * @param offset offset in bytes
 * @param buffer id of buffer
 */
void     GPU::setVertexPullerHead(VertexPullerID vao, uint32_t head, AttributeType type, uint64_t stride, uint64_t offset, BufferID buffer) {
    /// \todo Tato funkce nastaví jednu čtecí hlavu vertex pulleru.<br>
    /// Parametr "vao" vybírá tabulku s nastavením.<br>
    /// Parametr "head" vybírá čtecí hlavu vybraného vertex pulleru.<br>
    /// Parametr "type" nastaví typ atributu, který čtecí hlava čte. Tímto se vybere kolik dat v bajtech se přečte.<br>
    /// Parametr "stride" nastaví krok čtecí hlavy.<br>
    /// Parametr "offset" nastaví počáteční pozici čtecí hlavy.<br>
    /// Parametr "buffer" vybere buffer, ze kterého bude čtecí hlava číst.<br>
    std::list<VertexPullerSettings>::iterator item;
    for (item = VertexPullerList.begin(); item != VertexPullerList.end(); item++) {
        if (item->id == vao) {
            if (head < maxAttributes) {
                item->heads[head].type = type;
                item->heads[head].stride = stride;
                item->heads[head].offset = offset;
                item->heads[head].buf = buffer;
            }
            break;
        }
    }
}


/**
 * @brief This function sets vertex puller indexing.
 *
 * @param vao vertex puller id
 * @param type type of index
 * @param buffer buffer with indices
 */
void     GPU::setVertexPullerIndexing(VertexPullerID vao,IndexType type,BufferID buffer){
  /// \todo Tato funkce nastaví indexování vertex pulleru.
  /// Parametr "vao" vybírá tabulku s nastavením.<br>
  /// Parametr "type" volí typ indexu, který je uložený v bufferu.<br>
  /// Parametr "buffer" volí buffer, ve kterém jsou uloženy indexy.<br>
    std::list<VertexPullerSettings>::iterator item;
    for (item = VertexPullerList.begin(); item != VertexPullerList.end(); item++) {
        if (item->id == vao) {
            item->indexing.buf = buffer;
            item->indexing.type = type;
            item->indexing.enabled = true;
            break;
        }
    }
}

/**
 * @brief This function enables vertex puller's head.
 *
 * @param vao vertex puller 
 * @param head head id
 */
void     GPU::enableVertexPullerHead(VertexPullerID vao, uint32_t head) {
    /// \todo Tato funkce povolí čtecí hlavu daného vertex pulleru.<br>
    /// Pokud je čtecí hlava povolena, hodnoty z bufferu se budou kopírovat do atributu vrcholů vertex shaderu.<br>
    /// Parametr "vao" volí tabulku s nastavením vertex pulleru (vybírá vertex puller).<br>
    /// Parametr "head" volí čtecí hlavu.<br>
    std::list<VertexPullerSettings>::iterator item;
    for (item = VertexPullerList.begin(); item != VertexPullerList.end(); item++) {
        if (item->id == vao) {
            if (head < maxAttributes) {
                item->heads[head].enabled = true;
            }
            break;
        }
    }
}


/**
 * @brief This function disables vertex puller's head
 *
 * @param vao vertex puller id
 * @param head head id
 */
void     GPU::disableVertexPullerHead(VertexPullerID vao,uint32_t head){
  /// \todo Tato funkce zakáže čtecí hlavu daného vertex pulleru.<br>
  /// Pokud je čtecí hlava zakázána, hodnoty z bufferu se nebudou kopírovat do atributu vrcholu.<br>
  /// Parametry "vao" a "head" vybírají vertex puller a čtecí hlavu.<br>
    std::list<VertexPullerSettings>::iterator item;
    for (item = VertexPullerList.begin(); item != VertexPullerList.end(); item++) {
        if (item->id == vao) {
            if (head < maxAttributes) {
                item->heads[head].enabled = false;
            }
            break;
        }
    }
}

/**
 * @brief This function selects active vertex puller.
 *
 * @param vao id of vertex puller
 */
void     GPU::bindVertexPuller       (VertexPullerID vao){
  /// \todo Tato funkce aktivuje nastavení vertex pulleru.<br>
  /// Pokud je daný vertex puller aktivován, atributy z bufferů jsou vybírány na základě jeho nastavení.<br>
    std::list<VertexPullerSettings>::iterator item;
    for (item = VertexPullerList.begin(); item != VertexPullerList.end(); item++) {
        if (item->id == vao) {
            bindedVPid = item->id;
            break;
        }
    }
}

/**
 * @brief This function deactivates vertex puller.
 */
void     GPU::unbindVertexPuller     (){
  /// \todo Tato funkce deaktivuje vertex puller.
  /// To většinou znamená, že se vybere neexistující "emptyID" vertex puller.
    bindedVPid = emptyID;
}

/**
 * @brief This function tests if vertex puller exists.
 *
 * @param vao vertex puller
 *
 * @return true, if vertex puller "vao" exists
 */
bool     GPU::isVertexPuller         (VertexPullerID vao){
  /// \todo Tato funkce otestuje, zda daný vertex puller existuje.
  /// Pokud ano, funkce vrací true.
    std::list<VertexPullerSettings>::iterator item;
    for (item = VertexPullerList.begin(); item != VertexPullerList.end(); item++) {
        if (item->id == vao) {
            return true;
        }
    }
  return false;
}

/// @}

/** \addtogroup program_tasks 03. Implementace obslužných funkcí pro shader programy
 * @{
 */

/**
 * @brief This function creates new shader program.
 *
 * @return shader program id
 */
ProgramID        GPU::createProgram         (){
  /// \todo Tato funkce by měla vytvořit nový shader program.<br>
  /// Funkce vrací unikátní identifikátor nového proramu.<br>
  /// Program je seznam nastavení, které obsahuje: ukazatel na vertex a fragment shader.<br>
  /// Dále obsahuje uniformní proměnné a typ výstupních vertex attributů z vertex shaderu, které jsou použity pro interpolaci do fragment atributů.<br>
    Program newProgram;
    if (!ProgramList.empty())
        newProgram.id = ProgramList.back().id + 1;
    else
        newProgram.id = 1;

    ProgramList.push_back(newProgram);
    return newProgram.id;
  return emptyID;
}

/**
 * @brief This function deletes shader program
 *
 * @param prg shader program id
 */
void             GPU::deleteProgram         (ProgramID prg){
  /// \todo Tato funkce by měla smazat vybraný shader program.<br>
  /// Funkce smaže nastavení shader programu.<br>
  /// Identifikátor programu se stane volným a může být znovu využit.<br>
    std::list<Program>::iterator item;
    for (item = ProgramList.begin(); item != ProgramList.end(); item++) {
        if (item->id == prg) {
            item = ProgramList.erase(item);
            break;
        }
    }
}

/**
 * @brief This function attaches vertex and frament shader to shader program.
 *
 * @param prg shader program
 * @param vs vertex shader 
 * @param fs fragment shader
 */
void             GPU::attachShaders         (ProgramID prg,VertexShader vs,FragmentShader fs){
  /// \todo Tato funkce by měla připojít k vybranému shader programu vertex a fragment shader.
    for (std::list<Program>::iterator item = ProgramList.begin(); item != ProgramList.end(); item++) {
        if (item->id == prg) {
            item->VS = vs;
            item->FS = fs;
            break;
        }
    }
}

/**
 * @brief This function selects which vertex attributes should be interpolated during rasterization into fragment attributes.
 *
 * @param prg shader program
 * @param attrib id of attribute
 * @param type type of attribute
 */
void             GPU::setVS2FSType          (ProgramID prg,uint32_t attrib,AttributeType type){
  /// \todo tato funkce by měla zvolit typ vertex atributu, který je posílán z vertex shaderu do fragment shaderu.<br>
  /// V průběhu rasterizace vznikají fragment.<br>
  /// Fragment obsahují fragment atributy.<br>
  /// Tyto atributy obsahují interpolované hodnoty vertex atributů.<br>
  /// Tato funkce vybere jakého typu jsou tyto interpolované atributy.<br>
  /// Bez jakéhokoliv nastavení jsou atributy prázdne AttributeType::EMPTY<br>
    for (std::list<Program>::iterator item = ProgramList.begin(); item != ProgramList.end(); item++) {
        if (item->id == prg) {
            item->attributes[attrib] = type;
            break;
        }
    }
}

/**
 * @brief This function actives selected shader program
 *
 * @param prg shader program id
 */
void             GPU::useProgram            (ProgramID prg){
  /// \todo tato funkce by měla vybrat aktivní shader program.
    for (std::list<Program>::iterator item = ProgramList.begin(); item != ProgramList.end(); item++) {
        if (item->id == prg) {
            ActiveProgramID = item->id;
            break;
        }
    }
}

/**
 * @brief This function tests if selected shader program exists.
 *
 * @param prg shader program
 *
 * @return true, if shader program "prg" exists.
 */
bool             GPU::isProgram             (ProgramID prg){
  /// \todo tato funkce by měla zjistit, zda daný program existuje.<br>
  /// Funkce vráti true, pokud program existuje.<br>
    for (std::list<Program>::iterator item = ProgramList.begin(); item != ProgramList.end(); item++) {
        if (item->id == prg) {
            return true;
        }
    }
  return false;
}

/**
 * @brief This function sets uniform value (1 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void             GPU::programUniform1f(ProgramID prg, uint32_t uniformId, float     const& d) {
    /// \todo tato funkce by měla nastavit uniformní proměnnou shader programu.<br>
    /// Parametr "prg" vybírá shader program.<br>
    /// Parametr "uniformId" vybírá uniformní proměnnou. Maximální počet uniformních proměnných je uložen v programné \link maxUniforms \endlink.<br>
    /// Parametr "d" obsahuje data (1 float).<br>
    if (uniformId >= 0 && uniformId < maxUniforms) {
        for (std::list<Program>::iterator item = ProgramList.begin(); item != ProgramList.end(); item++) {
            if (item->id == prg) {
                item->un.uniform[uniformId].v1 = d;
                break;

            }
        }
    }
}

/**
 * @brief This function sets uniform value (2 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void             GPU::programUniform2f      (ProgramID prg,uint32_t uniformId,glm::vec2 const&d){
  /// \todo tato funkce dělá obdobnou věc jako funkce programUniform1f.<br>
  /// Místo 1 floatu nahrává 2 floaty.
    if (uniformId >= 0 && uniformId < maxUniforms) {
        for (std::list<Program>::iterator item = ProgramList.begin(); item != ProgramList.end(); item++) {
            if (item->id == prg) {
                item->un.uniform[uniformId].v2 = d;
                break;

            }
        }
    }
}

/**
 * @brief This function sets uniform value (3 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void             GPU::programUniform3f      (ProgramID prg, uint32_t uniformId, glm::vec3 const& d){
    /// \todo tato funkce dělá obdobnou věc jako funkce programUniform1f.<br>
    /// Místo 1 floatu nahrává 3 floaty.
    if (uniformId >= 0 && uniformId < maxUniforms) {
        for (std::list<Program>::iterator item = ProgramList.begin(); item != ProgramList.end(); item++) {
            if (item->id == prg) {
                item->un.uniform[uniformId].v3 = d;
                break;

            }
        }
    }
}

/**
 * @brief This function sets uniform value (4 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void             GPU::programUniform4f      (ProgramID prg, uint32_t uniformId, glm::vec4 const& d){
  /// \todo tato funkce dělá obdobnou věc jako funkce programUniform1f.<br>
  /// Místo 1 floatu nahrává 4 floaty.
    if (uniformId >= 0 && uniformId < maxUniforms) {
        for (std::list<Program>::iterator item = ProgramList.begin(); item != ProgramList.end(); item++) {
            if (item->id == prg) {
                item->un.uniform[uniformId].v4 = d;
                break;

            }
        }
    }
}

/**
 * @brief This function sets uniform value (4 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void             GPU::programUniformMatrix4f(ProgramID prg,uint32_t uniformId,glm::mat4 const&d){
  /// \todo tato funkce dělá obdobnou věc jako funkce programUniform1f.<br>
  /// Místo 1 floatu nahrává matici 4x4 (16 floatů).
    if (uniformId >= 0 && uniformId < maxUniforms) {
        for (std::list<Program>::iterator item = ProgramList.begin(); item != ProgramList.end(); item++) {
            if (item->id == prg) {
                item->un.uniform[uniformId].m4 = d;
                break;

            }
        }
    }
}

/// @}





/** \addtogroup framebuffer_tasks 04. Implementace obslužných funkcí pro framebuffer
 * @{
 */

/**
 * @brief This function creates framebuffer on GPU.
 *
 * @param width width of framebuffer
 * @param height height of framebuffer
 */
void GPU::createFramebuffer      (uint32_t width,uint32_t height){
  /// \todo Tato funkce by měla alokovat framebuffer od daném rozlišení.<br>
  /// Framebuffer se skládá z barevného a hloukového bufferu.<br>
  /// Buffery obsahují width x height pixelů.<br>
  /// Barevný pixel je složen z 4 x uint8_t hodnot - to reprezentuje RGBA barvu.<br>
  /// Hloubkový pixel obsahuje 1 x float - to reprezentuje hloubku.<br>
  /// Nultý pixel framebufferu je vlevo dole.<br>

    if (Width != 0 && Height != 0 && colorBuf != nullptr && depthBuf != nullptr)
    {
        resizeFramebuffer(width, height);
    }

    Width = width;
    Height = height;

    if (colorBuf == nullptr || depthBuf == nullptr) {
        colorBuf = (uint8_t*)malloc(sizeof(uint8_t) * 4 * width * height);
        depthBuf = (float*)malloc(sizeof(float) * width * height);
    }
    else 
    {
        colorBuf = (uint8_t*)realloc(colorBuf, sizeof(uint8_t) * 4 * width * height);
        depthBuf = (float*)realloc(depthBuf, sizeof(float) * width * height);
    }
    clear(0, 0, 0, 0);

}

/**
 * @brief This function deletes framebuffer.
 */
void GPU::deleteFramebuffer      (){
  /// \todo tato funkce by měla dealokovat framebuffer
    if (Width == 0 || Height == 0 || colorBuf == nullptr || depthBuf == nullptr)
        return;


    Width = 0;
    Height = 0;
    
}

/**
 * @brief This function resizes framebuffer.
 *
 * @param width new width of framebuffer
 * @param height new heght of framebuffer
 */
void     GPU::resizeFramebuffer(uint32_t width,uint32_t height){
  /// \todo Tato funkce by měla změnit velikost framebuffer.
    if (colorBuf == nullptr || depthBuf == nullptr || Width == 0 || Height == 0) {
        createFramebuffer(width, height);
        return;
    }

    Width = width;
    Height = height;



    if (colorBuf != nullptr)
        colorBuf = (uint8_t*)realloc(colorBuf, sizeof(uint8_t) * 4 * width * height);
    if (depthBuf != nullptr)
        depthBuf = (float*)realloc(depthBuf, sizeof(float) * width * height);
    
}

/**
 * @brief This function returns pointer to color buffer.
 *
 * @return pointer to color buffer
 */
uint8_t* GPU::getFramebufferColor  (){
  /// \todo Tato funkce by měla vrátit ukazatel na začátek barevného bufferu.<br>
    if (colorBuf != nullptr)
        return colorBuf;

  return nullptr;
}

/**
 * @brief This function returns pointer to depth buffer.
 *
 * @return pointer to dept buffer.
 */
float* GPU::getFramebufferDepth    (){
  /// \todo tato funkce by mla vrátit ukazatel na začátek hloubkového bufferu.<br>
    if (depthBuf != NULL)
        return depthBuf;
  return nullptr;
}

/**
 * @brief This function returns width of framebuffer
 *
 * @return width of framebuffer
 */
uint32_t GPU::getFramebufferWidth (){
  /// \todo Tato funkce by měla vrátit šířku framebufferu.
  return Width;
}

/**
 * @brief This function returns height of framebuffer.
 *
 * @return height of framebuffer
 */
uint32_t GPU::getFramebufferHeight(){
  /// \todo Tato funkce by měla vrátit výšku framebufferu.
  return Height;
}

/// @}

/** \addtogroup draw_tasks 05. Implementace vykreslovacích funkcí
 * Bližší informace jsou uvedeny na hlavní stránce dokumentace.
 * @{
 */

/**
 * @brief This functino clears framebuffer.
 *
 * @param r red channel
 * @param g green channel
 * @param b blue channel
 * @param a alpha channel
 */
void            GPU::clear                 (float r,float g,float b,float a){
  /// \todo Tato funkce by měla vyčistit framebuffer.<br>
  /// Barevný buffer vyčistí na barvu podle parametrů r g b a (0 - nulová intenzita, 1 a větší - maximální intenzita).<br>
  /// (0,0,0) - černá barva, (1,1,1) - bílá barva.<br>
  /// Hloubkový buffer nastaví na takovou hodnotu, která umožní rasterizaci trojúhelníka, který leží v rámci pohledového tělesa.<br>
  /// Hloubka by měla být tedy větší než maximální hloubka v NDC (normalized device coordinates).<br>
    if (r > 1)
        r = 1;
    if (g > 1)
        g = 1;
    if (b > 1)
        b = 1;
    if (a > 1)
        a = 1;
    

    for (unsigned i = 0; i < (Height * Width) * 4; i++) {

        colorBuf[i] = (uint8_t)(r * 255);
        colorBuf[++i] = g * 255;
        colorBuf[++i] = b * 255;
        colorBuf[++i] = a * 255;
        depthBuf[i / 4] = 1.1f;

    }
    
    
}



void            GPU::drawTriangles(uint32_t  nofVertices) {
    /// \todo Tato funkce vykreslí trojúhelníky podle daného nastavení.<br>
    /// Vrcholy se budou vybírat podle nastavení z aktivního vertex pulleru (pomocí bindVertexPuller).<br>
    /// Vertex shader a fragment shader se zvolí podle aktivního shader programu (pomocí useProgram).<br>
    /// Parametr "nofVertices" obsahuje počet vrcholů, který by se měl vykreslit (3 pro jeden trojúhelník).<br>


      // Getting binded puller
    std::list<VertexPullerSettings>::iterator VP;
    for (VP = VertexPullerList.begin(); VP != VertexPullerList.end(); VP++) {
        if (VP->id == bindedVPid) {
            break;
        }
    }


    // Getting active program
    std::list<Program>::iterator P;
    for (P = ProgramList.begin(); P != ProgramList.end(); P++) {
        if (P->id == ActiveProgramID) {
            break;
        }
    }

    std::vector<OutVertex> outVertexes{ size_t(nofVertices) };


    for (uint32_t inVertexID = 0; inVertexID < nofVertices; inVertexID++) {
        InVertex inVertex;

        if (VP->indexing.enabled)
        {
            for (std::list<Buffer>::iterator buf = BufferList.begin(); buf != BufferList.end(); buf++) {
                if (buf->id == VP->indexing.buf) {
                    switch (VP->indexing.type)
                    {
                        uint8_t* data;
                        uint16_t* data2;
                        uint32_t* data3;
                        case IndexType::UINT8:
                            data = (uint8_t*)buf->data;
                            inVertex.gl_VertexID = (uint32_t)(data[inVertexID]);
                            break;
                        case IndexType::UINT16:
                            data2 = (uint16_t*)buf->data;
                            inVertex.gl_VertexID = (uint32_t)(data2[inVertexID]);
                            break;
                        case IndexType::UINT32:
                            data3 = (uint32_t*)buf->data;
                            inVertex.gl_VertexID = (uint32_t)(data3[inVertexID]);
                            break;
                    }
                    
                    break;
                }
            }
        }
        else
            inVertex.gl_VertexID = inVertexID;


        for (int i = 0; i < maxAttributes; i++) {
            if (VP->heads[i].enabled)
                for (std::list<Buffer>::iterator buf = BufferList.begin(); buf != BufferList.end(); buf++) {
                    if (buf->id == VP->heads[i].buf) {
                        switch (VP->heads[i].type)
                        {
                            float* data;
                            glm::vec2* data2;
                            glm::vec3* data3;
                            glm::vec4* data4;

                        case AttributeType::FLOAT:
                            data = (float*)malloc(sizeof(float));
                            if (data != nullptr) {
                                memcpy(data, (uint8_t*)buf->data + (inVertex.gl_VertexID * VP->heads[i].stride + VP->heads[i].offset), sizeof(data));
                                inVertex.attributes[i].v1 = *data;
                            }
                            break;

                        case AttributeType::VEC2:
                            data2 = (glm::vec2*)malloc(sizeof(glm::vec2));
                            if (data2 != nullptr) {
                                memcpy(data2, (uint8_t*)buf->data + (inVertex.gl_VertexID * VP->heads[i].stride + VP->heads[i].offset), sizeof(float) * 2);
                                inVertex.attributes[i].v2 = *data2;
                            }
                            break;

                        case AttributeType::VEC3:
                            data3 = (glm::vec3*)malloc(sizeof(glm::vec3));
                            if (data3 != nullptr) {
                                memcpy(data3, (uint8_t*)buf->data + (inVertex.gl_VertexID * VP->heads[i].stride + VP->heads[i].offset), sizeof(float) * 3);
                                inVertex.attributes[i].v3 = *data3;
                            }
                            break;

                        case AttributeType::VEC4:
                            data4 = (glm::vec4*)malloc(sizeof(glm::vec4));
                            if (data4 != nullptr) {
                                memcpy(data4, (uint8_t*)buf->data + (inVertex.gl_VertexID * VP->heads[i].stride + VP->heads[i].offset), sizeof(float) * 4);
                                inVertex.attributes[i].v4 = *data4;
                            }
                            break;

                        default: break;
                        }
                        break;
                    }
                }
        }

        OutVertex outVertex;
        Uniforms const& uniforms = P->un;
        P->VS(outVertex, inVertex, uniforms);
        outVertexes.push_back(outVertex);
    }

   for (size_t i = 0; i < outVertexes.size(); i++) {
        auto& a = outVertexes[i];
        auto& b = outVertexes[++i];
        auto& c = outVertexes[++i];
        trianglesClipping(a, b, c);
    }

    return;
}


/**
 * @brief Triangle clipping
 *
 * @param a - first vertex of the triangle
 * @param b - second vertex of the triangle
 * @param c - third vertex of the triangle
 */
void GPU::trianglesClipping(OutVertex& a, OutVertex& b, OutVertex& c) {

   if (a.gl_Position[0] > a.gl_Position[3] &&
        b.gl_Position[0] > b.gl_Position[3] &&
        c.gl_Position[0] > c.gl_Position[3])
        return;
    if (a.gl_Position[0] < -a.gl_Position[3] &&
        b.gl_Position[0] < -b.gl_Position[3] &&
        c.gl_Position[0] < -c.gl_Position[3])
        return;
    if (a.gl_Position[1] > a.gl_Position[3] &&
        b.gl_Position[1] > b.gl_Position[3] &&
        c.gl_Position[1] > c.gl_Position[3])
        return;
    if (a.gl_Position[1] < -a.gl_Position[3] &&
        b.gl_Position[1] < -b.gl_Position[3] &&
        c.gl_Position[1] < -c.gl_Position[3])
        return;
    if (a.gl_Position[2] > a.gl_Position[3] &&
        b.gl_Position[2] > b.gl_Position[3] &&
        c.gl_Position[2] > c.gl_Position[3])
        return;
    if (a.gl_Position[2] < -a.gl_Position[3] &&
        b.gl_Position[2] < -b.gl_Position[3] &&
        c.gl_Position[2] < -c.gl_Position[3])
        return;
    
    auto Clipping1 = [this](OutVertex& a, OutVertex& b, OutVertex& c) {
        float A = (-a.gl_Position[3] - a.gl_Position[2]) / (b.gl_Position[3] - a.gl_Position[3] + b.gl_Position[2] - a.gl_Position[2]);
        float B = (-a.gl_Position[3] - a.gl_Position[2]) / (c.gl_Position[3] - a.gl_Position[3] + c.gl_Position[2] - a.gl_Position[2]);

        auto a1 = Interpolation(a, b, A);
        auto a2 = Interpolation(a, c, B);

        auto a1clone = a1;
        auto cclone = c;

        Drawing(a1, b, c);
        Drawing(a2, a1clone, cclone);
    };

    auto Clipping2 = [this](OutVertex& a, OutVertex& b, OutVertex& c) {
        float A = (-a.gl_Position[3] - a.gl_Position[2]) / (c.gl_Position[3] - a.gl_Position[3] + c.gl_Position[2] - a.gl_Position[2]);
        float B = (-b.gl_Position[3] - b.gl_Position[2]) / (c.gl_Position[3] - b.gl_Position[3] + c.gl_Position[2] - b.gl_Position[2]);

        a = Interpolation(a, c, A);
        b = Interpolation(b, c, B);

        Drawing(a, b, c);
    };
    
    if (a.gl_Position[2] < -a.gl_Position[3]) {

        if (b.gl_Position[2] < -b.gl_Position[3])
            Clipping2(a, b, c);

        else if (c.gl_Position[2] < -c.gl_Position[3])
            Clipping2(a, c, b);

        else
            Clipping1(a, b, c);

    }
    else if (b.gl_Position[2] < -b.gl_Position[3]) {
        if (c.gl_Position[2] < -c.gl_Position[3])
            Clipping2(b, c, a);

        else
            Clipping1(b, a, c);
    }
    else if (c.gl_Position[2] < -c.gl_Position[3])
        Clipping1(c, a, b);
    else
        Drawing(a, b, c);

}


/**
 * @brief helping interpolation
 *
 * @param a - first vertex of the triangle
 * @param b - second vertex of the triangle
 * @param f - actually, I don't know what is it
 */
OutVertex GPU::Interpolation(OutVertex& a, OutVertex& b, float f) {
    OutVertex res = a;
    res.gl_Position = a.gl_Position + f * (b.gl_Position - a.gl_Position);

    return res;
}


void GPU::Drawing(OutVertex& a, OutVertex& b, OutVertex& c) {
    
    // Do post processes
    postProcesses(a, b, c);

    // Find triangle's top, bottom and height
    float triangleTop = getTraingleTop(a, b, c);
    float triangleBottom = getTraingleBottom(a, b, c);

    float triangleHeight = triangleTop - triangleBottom;
    
    // Make array borders with sructure:
    // Borders[y] = x1
    // Borders[y] = x2
    // x1 and x2 - start and end of lines, which will be drawn
    glm::vec2* Lines = (glm::vec2*)malloc(sizeof(glm::vec2) * (size_t)ceil(triangleHeight));
    glm::vec2 emptyVec2; emptyVec2[0] = -1; emptyVec2[1] = -1;
    for (int i = 0; i < triangleHeight; i++) Lines[i] = emptyVec2;


    // Getting lines
    Line(a, b, Lines, a, b, c);
    Line(b, c, Lines, a, b, c);
    Line(c, a, Lines, a, b, c);

    // Getting active program
    std::list<Program>::iterator P;
    for (P = ProgramList.begin(); P != ProgramList.end(); P++) {
        if (P->id == ActiveProgramID) {
            break;
        }
    }

    float y = triangleBottom + 0.5;
    
    for (y; y < triangleTop; y++) {
        if (y < 0 || y > Height)
            continue;

        float x = -1;
        float x2 = -1;

        x = Lines[int(y - 0.5 - triangleBottom)][0];
      
        if (x > Width || x < 0)
            continue;

        x2 = Lines[int(y - 0.5 - triangleBottom)][1];

        if (x2 > Width || x2 < 0)
            continue;

        if (x > x2) 
            swapFloat(x, x2);
        
        
        x += 0.5;
       
        
        for (x; x <= x2; x++) {
            
            float h2 = (x - a.gl_Position[0]) * (b.gl_Position[1] - a.gl_Position[1]) - (y - a.gl_Position[1]) * (b.gl_Position[0] - a.gl_Position[0]);
            float h0 = (x - b.gl_Position[0]) * (c.gl_Position[1] - b.gl_Position[1]) - (y - b.gl_Position[1]) * (c.gl_Position[0] - b.gl_Position[0]);
            float h1 = (x - c.gl_Position[0]) * (a.gl_Position[1] - c.gl_Position[1]) - (y - c.gl_Position[1]) * (a.gl_Position[0] - c.gl_Position[0]);


            h0 = h0 / (Width * Height);
            h1 = h1 / (Width * Height);
            h2 = h2 / (Width * Height);

            float z = ((a.gl_Position[2] * h0) / a.gl_Position[3] + (b.gl_Position[2] * h1) / b.gl_Position[3] + (c.gl_Position[2] * h2) / c.gl_Position[3]) / (h0 / a.gl_Position[3] + h1 / b.gl_Position[3] + h2 / c.gl_Position[3]);           
            
            // Depth test
            float depth = depthBuf[((int)y * Width + (int)x)];
            if (!(depth > z))
                continue;

            InFragment inFragment;
            inFragment.gl_FragCoord[0] = x;
            inFragment.gl_FragCoord[1] = y;
            inFragment.gl_FragCoord[2] = z;
            for (int i = 0; i < maxAttributes; i++) {

                switch (P->attributes[i])
                {
                case AttributeType::FLOAT:
                    inFragment.attributes[i].v1 = ((a.attributes[i].v1 * h0) / a.gl_Position[3] + (b.attributes[i].v1 * h1) / b.gl_Position[3] + (c.attributes[i].v1 * h2) / c.gl_Position[3]) / (h0 / a.gl_Position[3] + h1 / b.gl_Position[3] + h2 / c.gl_Position[3]);
                    break;
                case AttributeType::VEC2:
                    inFragment.attributes[i].v2 = ((a.attributes[i].v2 * h0) / a.gl_Position[3] + (b.attributes[i].v2 * h1) / b.gl_Position[3] + (c.attributes[i].v2 * h2) / c.gl_Position[3]) / (h0 / a.gl_Position[3] + h1 / b.gl_Position[3] + h2 / c.gl_Position[3]);
                    break;
                case AttributeType::VEC3:
                    inFragment.attributes[i].v3 = ((a.attributes[i].v3 * h0) / a.gl_Position[3] + (b.attributes[i].v3 * h1) / b.gl_Position[3] + (c.attributes[i].v3 * h2) / c.gl_Position[3]) / (h0 / a.gl_Position[3] + h1 / b.gl_Position[3] + h2 / c.gl_Position[3]);
                    break;
                case AttributeType::VEC4:
                    inFragment.attributes[i].v4 = ((a.attributes[i].v4 * h0) / a.gl_Position[3] + (b.attributes[i].v4 * h1) / b.gl_Position[3] + (c.attributes[i].v4 * h2) / c.gl_Position[3]) / (h0 / a.gl_Position[3] + h1 / b.gl_Position[3] + h2 / c.gl_Position[3]);
                    break;
                }

            }
            putPixel(inFragment);
            
        }
    }   
}



/**
 * @brief getting lines coordinates for drawing 
 *
 * @param a - start vertex of the line
 * @param b - end vertex of the line
 * @param Lines - array which be written by coordinates of borders
 * @param v0 - first vertex of the triangle
 * @param v1 - first vertex of the triangle
 * @param v2 - first vertex of the triangle
 */
void GPU::Line(OutVertex a, OutVertex b, glm::vec2 *Lines, OutVertex& v0, OutVertex& v1, OutVertex& v2) {
    
    // Getting active program
    std::list<Program>::iterator P;
    for (P = ProgramList.begin(); P != ProgramList.end(); P++) {
        if (P->id == ActiveProgramID) {
            break;
        }
    }

    bool steep = (abs(b.gl_Position[1] - a.gl_Position[1]) > abs(b.gl_Position[0] - a.gl_Position[0]));

    if (steep) {
        swapFloat(a.gl_Position[0], a.gl_Position[1]);
        swapFloat(b.gl_Position[0], b.gl_Position[1]);
    }

    if (a.gl_Position[0] > b.gl_Position[0])
        swapVertex(a, b);


    float dx = b.gl_Position[0] - a.gl_Position[0];
    float dy = abs(b.gl_Position[1] - a.gl_Position[1]);

    float err = dx / 2;

    float ystep = (a.gl_Position[1] < b.gl_Position[1]) ? 1 : -1;
    float y = a.gl_Position[1]; 
    

    for (float x = a.gl_Position[0]; x < b.gl_Position[0]; x++) {
        if (steep)
            swapFloat(x, y);

        if (y < 0 || y > Height)
            continue;
        if (x < 0 || x > Width)
            continue;

        float triangleBottom = getTraingleBottom(v0, v1, v2);

            if (Lines[int(y - triangleBottom)][0] == -1)
                Lines[int(y - triangleBottom)][0] = x;

            else if (Lines[int(y - triangleBottom)][1] == -1)
                Lines[int(y - triangleBottom)][1] = x;

            else {
                Lines[int(y - triangleBottom)][0] = Lines[int(y - triangleBottom)][0] < Lines[int(y - triangleBottom)][1] ? Lines[int(y - triangleBottom)][0] : Lines[int(y - triangleBottom)][1];
                if (x < Lines[int(y - triangleBottom)][0])
                    Lines[int(y - triangleBottom)][0] = x;
                if (x > Lines[int(y - triangleBottom)][1])
                    Lines[int(y - triangleBottom)][1] = x;
        }

        err -= dy;

        if (err < 0)
        {
            y += ystep;
            err += dx;
        }

        if (steep)
            swapFloat(x, y);

    }

}

 void GPU::swapVertex(OutVertex& a, OutVertex& b)
{
     OutVertex t;
     t = a;
     a = b;
     b = t;
}


 void GPU::swapFloat(float &a, float &b) {
     float t = a;
     a = b;
     b = t;
}


 void GPU::putPixel(InFragment inFragment) {
     std::list<Program>::iterator P;
     for (P = ProgramList.begin(); P != ProgramList.end(); P++) {
         if (P->id == ActiveProgramID) {
             break;
         }
     }

     int x = inFragment.gl_FragCoord[0];
     int y = inFragment.gl_FragCoord[1];
     int z = inFragment.gl_FragCoord[2];
     OutFragment outFragment;
     P->FS(outFragment, inFragment, P->un);

     colorBuf[((int)y * Width + (int)x) * 4] = outFragment.gl_FragColor[0] * 255;
     colorBuf[((int)y * Width + (int)x) * 4 + 1] = outFragment.gl_FragColor[1] * 255;
     colorBuf[((int)y * Width + (int)x) * 4 + 2] = outFragment.gl_FragColor[2] * 255;
     colorBuf[((int)y * Width + (int)x) * 4 + 3] = outFragment.gl_FragColor[3] * 255;
     depthBuf[((int)y * Width + (int)x)] = z;
 }

 float GPU::getTraingleTop(OutVertex& a, OutVertex& b, OutVertex& c) {
     float triangleTop = a.gl_Position[1] > b.gl_Position[1] ? a.gl_Position[1] : b.gl_Position[1];
     triangleTop = triangleTop > c.gl_Position[1] ? triangleTop : c.gl_Position[1];

     return triangleTop;
 }


 float GPU::getTraingleBottom(OutVertex& a, OutVertex& b, OutVertex& c) {
     float triangleBottom = a.gl_Position[1] < b.gl_Position[1] ? a.gl_Position[1] : b.gl_Position[1];
     triangleBottom = triangleBottom < c.gl_Position[1] ? triangleBottom : c.gl_Position[1];

     return triangleBottom;
 }

 void GPU::postProcesses(OutVertex& a, OutVertex& b, OutVertex& c) {
     a.gl_Position[0] = a.gl_Position[0] / a.gl_Position[3];
     a.gl_Position[1] = a.gl_Position[1] / a.gl_Position[3];
     a.gl_Position[2] = a.gl_Position[2] / a.gl_Position[3];

     b.gl_Position[0] = b.gl_Position[0] / b.gl_Position[3];
     b.gl_Position[1] = b.gl_Position[1] / b.gl_Position[3];
     b.gl_Position[2] = b.gl_Position[2] / b.gl_Position[3];

     c.gl_Position[0] = c.gl_Position[0] / c.gl_Position[3];
     c.gl_Position[1] = c.gl_Position[1] / c.gl_Position[3];
     c.gl_Position[2] = c.gl_Position[2] / c.gl_Position[3];


     a.gl_Position[0] = (a.gl_Position[0] + 1) * Width / 2;
     a.gl_Position[1] = (a.gl_Position[1] + 1) * Height / 2;

     b.gl_Position[0] = (b.gl_Position[0] + 1) * Width / 2;
     b.gl_Position[1] = (b.gl_Position[1] + 1) * Height / 2;

     c.gl_Position[0] = (c.gl_Position[0] + 1) * Width / 2;
     c.gl_Position[1] = (c.gl_Position[1] + 1) * Height / 2;
 }


/// @}

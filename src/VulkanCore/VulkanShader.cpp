#include "rad/VulkanCore/VulkanShader.h"
#include "rad/VulkanCore/VulkanDevice.h"
#include "rad/Core/File.h"

#include "shaderc/shaderc.hpp"

class VulkanShaderPrivate : public RefCounted<VulkanShaderPrivate>
{
public:
    VulkanShaderPrivate()
    {
    }
    ~VulkanShaderPrivate()
    {
    }

    void SetStage(VkShaderStageFlagBits stage) { m_stage = stage; }
    void SetFileName(const std::string_view fileName) { m_fileName = fileName; }
    void SetEntryPoint(const std::string_view entryPoint) { m_entryPoint = entryPoint; }
    void SetMacros(ArrayRef<ShaderMacro> macros) { m_macros = { macros.begin(), macros.end() }; }
    void SetLanguage(ShaderLanguage language) { m_language = language; }
    void SetIncludeDir(const std::string_view includeDir) { m_includeDir = includeDir; }
    void SetBinary(ArrayRef<uint32_t> binary);

    VkShaderStageFlagBits GetStage() const { return m_stage; }
    std::string GetFileName() const { return m_fileName; }
    const char* GetEntryPoint() const { return m_entryPoint.data(); }
    const std::vector<ShaderMacro>& GetMacros() const { return m_macros; }
    ShaderLanguage GetLanguage() const { return m_language; }

    bool Compile(const std::string_view fileName, const std::string_view source);
    bool LoadBinaryFromFile(const std::string_view fileName);
    bool IsValid() const;

    const std::vector<uint32_t>& GetBinary() const { return m_binary; }
    const char* GetLog() const { return m_log.c_str(); }

private:
    shaderc::Compiler m_compiler;
    shaderc::CompileOptions m_options;

    VkShaderStageFlagBits m_stage = VK_SHADER_STAGE_ALL;
    std::string m_fileName;
    std::string m_source;
    std::string m_entryPoint = "main";
    std::vector<ShaderMacro> m_macros;
    ShaderLanguage m_language = ShaderLanguage::GLSL;

    std::string m_includeDir;

    std::vector<uint32_t> m_binary;
    std::string m_log;

}; // class VulkanShaderPrivate

class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface
{
public:
    ShaderIncluder(std::string_view includeDir)
    {
        m_includeDir = includeDir;
    }

    struct IncludeInfo
    {
        std::string absolutePath;
        std::string content;
    };

    // Handles shaderc_include_resolver_fn callbacks.
    virtual shaderc_include_result* GetInclude(
        const char*             requestedSource,
        shaderc_include_type    type,
        const char*             requestingSource,
        size_t                  include_depth) override
    {
        shaderc_include_result* pIncludeResult = new shaderc_include_result();
        IncludeInfo* pIncludeInfo = new IncludeInfo();

        try
        {
            if (type == shaderc_include_type::shaderc_include_type_relative)
            {
                pIncludeInfo->absolutePath =
                    (std::filesystem::absolute(requestingSource).remove_filename() / requestedSource).string();
            }
            else if (type == shaderc_include_type::shaderc_include_type_standard)
            {
                pIncludeInfo->absolutePath = m_includeDir + requestingSource;
            }
            pIncludeInfo->content = File::ReadAll(pIncludeInfo->absolutePath);

            pIncludeResult->source_name = pIncludeInfo->absolutePath.data();
            pIncludeResult->source_name_length = pIncludeInfo->absolutePath.length();
            pIncludeResult->content = pIncludeInfo->content.data();
            pIncludeResult->content_length = pIncludeInfo->content.size();
            pIncludeResult->user_data = pIncludeInfo;
            return pIncludeResult;
        }
        catch (...)
        {
            ReleaseInclude(pIncludeResult);
            return nullptr;
        }
    }

    // Handles shaderc_include_result_release_fn callbacks.
    virtual void ReleaseInclude(shaderc_include_result* data) override
    {
        if (data)
        {
            if (IncludeInfo* pIncludeInfo = (IncludeInfo*)data->user_data)
            {
                delete pIncludeInfo;
            }
            delete data;
        }
    }

private:
    std::string m_includeDir;

}; // class ShaderIncluder

void VulkanShaderPrivate::SetBinary(ArrayRef<uint32_t> binary)
{
    m_binary = binary;
}

bool VulkanShaderPrivate::Compile(const std::string_view fileName, const std::string_view source)
{
    m_fileName = fileName;
    m_source = source;

    shaderc::SpvCompilationResult result;

    shaderc_shader_kind shaderKind = shaderc_glsl_infer_from_source;
    switch (m_stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT:                     shaderKind = shaderc_vertex_shader;            break;
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:       shaderKind = shaderc_tess_control_shader;      break;
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:    shaderKind = shaderc_tess_evaluation_shader;   break;
    case VK_SHADER_STAGE_GEOMETRY_BIT:                   shaderKind = shaderc_geometry_shader;          break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:                   shaderKind = shaderc_fragment_shader;          break;
    case VK_SHADER_STAGE_COMPUTE_BIT:                    shaderKind = shaderc_compute_shader;           break;
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:                 shaderKind = shaderc_raygen_shader;            break;
    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:                shaderKind = shaderc_anyhit_shader;            break;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:            shaderKind = shaderc_closesthit_shader;        break;
    case VK_SHADER_STAGE_MISS_BIT_KHR:                   shaderKind = shaderc_miss_shader;              break;
    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:           shaderKind = shaderc_intersection_shader;      break;
    case VK_SHADER_STAGE_CALLABLE_BIT_KHR:               shaderKind = shaderc_callable_shader;          break;
    case VK_SHADER_STAGE_TASK_BIT_NV:                    shaderKind = shaderc_task_shader;              break;
    case VK_SHADER_STAGE_MESH_BIT_NV:                    shaderKind = shaderc_mesh_shader;              break;
    }

    m_options.SetOptimizationLevel(shaderc_optimization_level_performance);
    for (const ShaderMacro& macro : m_macros)
    {
        m_options.AddMacroDefinition(macro.m_name.data(), macro.m_definition.data());
    }

    ShaderMacro shaderStageMacro;
    switch (m_stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT:                    shaderStageMacro.m_name = "SHADER_STAGE_VERTEX";                break;
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:      shaderStageMacro.m_name = "SHADER_STAGE_TESSELLATION_CONTROL";  break;
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:   shaderStageMacro.m_name = "SHADER_STAGE_TESSELLATION_EVALUATION";   break;
    case VK_SHADER_STAGE_GEOMETRY_BIT:                  shaderStageMacro.m_name = "SHADER_STAGE_GEOMETRY";              break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:                  shaderStageMacro.m_name = "SHADER_STAGE_FRAGMENT";              break;
    case VK_SHADER_STAGE_COMPUTE_BIT:                   shaderStageMacro.m_name = "SHADER_STAGE_COMPUTE";               break;
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:                shaderStageMacro.m_name = "SHADER_STAGE_RAY_GEN";               break;
    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:               shaderStageMacro.m_name = "SHADER_STAGE_RAY_ANY_HIT";           break;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:           shaderStageMacro.m_name = "SHADER_STAGE_RAY_CLOSEST_HIT";       break;
    case VK_SHADER_STAGE_MISS_BIT_KHR:                  shaderStageMacro.m_name = "SHADER_STAGE_RAY_MISS";              break;
    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:          shaderStageMacro.m_name = "SHADER_STAGE_RAY_INTERSECTION";      break;
    case VK_SHADER_STAGE_CALLABLE_BIT_KHR:              shaderStageMacro.m_name = "SHADER_STAGE_CALLABLE";              break;
    case VK_SHADER_STAGE_TASK_BIT_NV:                   shaderStageMacro.m_name = "SHADER_STAGE_TASK";                  break;
    case VK_SHADER_STAGE_MESH_BIT_NV:                   shaderStageMacro.m_name = "SHADER_STAGE_MESH";                  break;
    }
    shaderStageMacro.m_definition = "1";
    m_options.AddMacroDefinition(shaderStageMacro.m_name.data(), shaderStageMacro.m_definition.data());

    m_options.SetIncluder(std::make_unique<ShaderIncluder>(m_includeDir));

    if (m_language == ShaderLanguage::GLSL)
    {
        if (!m_entryPoint.empty() && m_entryPoint != "main")
        {
            m_options.AddMacroDefinition(m_entryPoint.data(), "main");
        }
        m_options.SetSourceLanguage(shaderc_source_language_glsl);
        result = m_compiler.CompileGlslToSpv(m_source.data(), shaderKind, m_fileName.c_str(), m_options);
    }
    else if (m_language == ShaderLanguage::HLSL)
    {
        m_options.SetSourceLanguage(shaderc_source_language_hlsl);
        result = m_compiler.CompileGlslToSpv(m_source.data(), shaderKind, m_fileName.c_str(), m_options);
    }

    m_log = result.GetErrorMessage().c_str();
    if (result.GetCompilationStatus() == shaderc_compilation_status_success)
    {
        m_binary = { result.begin(), result.end() };
#if 0
        shaderc::AssemblyCompilationResult assembly = m_compiler.CompileGlslToSpvAssembly(m_source.data(), shaderKind, m_fileName.c_str(), m_options);
        File file;
        if (file.Open(m_fileName + ".asm.txt", FileOpenWrite))
        {
            file.Write(assembly.cbegin(), assembly.cend() - assembly.cbegin());
        }
#endif
        return true;
    }
    else
    {
        return false;
    }
}

bool VulkanShaderPrivate::LoadBinaryFromFile(const std::string_view fileName)
{
    File binaryFile;
    if (binaryFile.Open(fileName, FileOpenRead | FileOpenBinary))
    {
        uintmax_t fileSize = binaryFile.Size();
        if (fileSize % sizeof(uint32_t) == 0)
        {
            m_binary.resize(fileSize / sizeof(uint32_t));
            binaryFile.Read(reinterpret_cast<char*>(m_binary.data()), fileSize);
            return true;
        }
    }
    return false;
}

bool VulkanShaderPrivate::IsValid() const
{
    return (!m_binary.empty() && (m_binary[0] == 0x07230203));
}

VulkanShader::VulkanShader()
{
    d_ptr = new VulkanShaderPrivate();
}

VulkanShader::VulkanShader(VkShaderStageFlagBits stage) :
    VulkanShader()
{
    d_ptr->SetStage(stage);
}

VulkanShader::~VulkanShader()
{
}

VulkanShader& VulkanShader::SetStage(VkShaderStageFlagBits stage)
{
    d_ptr->SetStage(stage);
    return *this;
}

VulkanShader& VulkanShader::SetEntryPoint(const std::string_view entryPoint)
{
    d_ptr->SetEntryPoint(entryPoint);
    return *this;
}

VulkanShader& VulkanShader::SetMacros(ArrayRef<ShaderMacro> macros)
{
    d_ptr->SetMacros(macros);
    return *this;
}

VulkanShader& VulkanShader::SetLanguage(ShaderLanguage language)
{
    d_ptr->SetLanguage(language);
    return *this;
}

VulkanShader& VulkanShader::SetIncludeDir(const std::string_view includeDir)
{
    d_ptr->SetIncludeDir(includeDir);
    return *this;
}

VulkanShader& VulkanShader::SetBinary(const ArrayRef<uint32_t> binary)
{
    d_ptr->SetBinary(binary);
    return *this;
}

VkShaderStageFlagBits VulkanShader::GetStage() const
{
    return d_ptr->GetStage();
}

std::string VulkanShader::GetFileName() const
{
    return d_ptr->GetFileName();
}

const char* VulkanShader::GetEntryPoint() const
{
    return d_ptr->GetEntryPoint();
}

const std::vector<ShaderMacro>& VulkanShader::GetMacros() const
{
    return d_ptr->GetMacros();
}

ShaderLanguage VulkanShader::GetLanguage() const
{
    return d_ptr->GetLanguage();
}

bool VulkanShader::Compile(
    const std::string_view fileName, const std::string_view source,
    const std::string_view entryPoint,
    ArrayRef<ShaderMacro> macros)
{
    d_ptr->SetEntryPoint(entryPoint);
    d_ptr->SetMacros(macros);
    return d_ptr->Compile(fileName, source);
}

bool VulkanShader::Compile(const std::string_view fileName, const std::string_view source)
{
    return d_ptr->Compile(fileName, source);
}

bool VulkanShader::LoadBinaryFromFile(const std::string_view fileName)
{
    return d_ptr->LoadBinaryFromFile(fileName);
}

bool VulkanShader::IsValid() const
{
    return d_ptr->IsValid();
}

const std::vector<uint32_t>& VulkanShader::GetBinary() const
{
    return d_ptr->GetBinary();
}

const char* VulkanShader::GetLog() const
{
    return d_ptr->GetLog();
}

VulkanShaderModule::VulkanShaderModule(Ref<VulkanDevice> device, const VkShaderModuleCreateInfo& createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateShaderModule(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

VulkanShaderModule::~VulkanShaderModule()
{
    m_device->GetFunctionTable()->
        vkDestroyShaderModule(m_device->GetHandle(), m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
}

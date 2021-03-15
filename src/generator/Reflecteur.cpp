#include <spirv_cross/spirv_cross.hpp>

int main() {
    return 0;
}

// // TODO : use it to generate layouts !
// void _reflectShaderFile(const cmrc::file& shaderBinary) {
//     using namespace spirv_cross;

//     Compiler comp(
//         reinterpret_cast<const uint32_t*>(shaderBinary.cbegin()), 
//         shaderBinary.size() / sizeof(uint32_t)
//     );
    
//     // The SPIR-V is now parsed, and we can perform reflection on it.
//     ShaderResources resources = comp.get_shader_resources();
//     for (auto &u : resources.uniform_buffers) {
//         uint32_t set = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
//         uint32_t binding = comp.get_decoration(u.id, spv::DecorationBinding);
//         std::printf("Found UBO %s at set = %u, binding = %u!\n", u.name.c_str(), set, binding);
//     }
// }
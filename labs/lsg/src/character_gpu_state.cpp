#include "rengine/lsg/character_gpu_state.hpp"

namespace rengine::lsg {

CharacterGpuStateV0 pack_character_gpu_state(
    const CharacterGenomeV0& genome,
    const CarrierFaceFieldMetadataV0& metadata,
    const PhysiologyState& physiology) noexcept {
  CharacterGpuStateV0 gpu{};
  const std::int16_t raw[20] = {
      genome.face.skull_width, genome.face.skull_height,
      genome.face.face_length, genome.face.forehead_height,
      genome.face.brow_depth, genome.face.eye_spacing,
      genome.face.eye_size, genome.face.eye_tilt,
      genome.face.nose_length, genome.face.nose_width,
      genome.face.nose_projection, genome.face.cheekbone_width,
      genome.face.cheek_fullness, genome.face.jaw_width,
      genome.face.chin_width, genome.face.chin_projection,
      genome.face.mouth_width, genome.face.upper_lip_fullness,
      genome.face.lower_lip_fullness, genome.face.lip_projection};
  float* face_vectors[5] = {
      gpu.identity.face0, gpu.identity.face1, gpu.identity.face2,
      gpu.identity.face3, gpu.identity.face4};
  for (std::size_t i = 0; i < 20; ++i)
    face_vectors[i / 4][i % 4] = decode_snorm16(raw[i]);

  gpu.identity.basis0[0] = metadata.head_pivot_y;
  gpu.identity.basis0[1] = metadata.face_half_width_m;
  gpu.identity.basis0[2] = metadata.face_half_height_m;
  gpu.identity.basis0[3] = metadata.face_depth_m;
  gpu.identity.basis1[0] = metadata.eye_center_x_abs_m;
  gpu.identity.basis1[1] = metadata.eye_center_y_m;
  gpu.identity.basis1[2] = static_cast<float>(metadata.version);

  gpu.skin = pack_skin_material_gpu(derive_skin_phenotype(genome, physiology));
  return gpu;
}

} // namespace rengine::lsg

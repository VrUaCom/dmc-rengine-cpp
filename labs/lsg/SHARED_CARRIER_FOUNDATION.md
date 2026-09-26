# Shared Carrier / Character Profile Foundation

Status: **STRUCTURAL_IMPLEMENTATION**

## Rule

Character identity is not a mesh allocation.

The runtime separates:

- **Carrier** — shared topology/runtime geometry resource.
- **Character Profile** — compact identity configuration that references a carrier.
- **Human DNA** — body, face, skin, eye, microdetail and physiology parameters.

Current built-in mapping:

| Profile | Carrier |
|---|---|
| Male Base | male-base |
| Female Base | female-base |
| Ada | female-base |

Ada therefore owns no duplicate female RMS0 body or eye assets. Her identity is produced from the shared female carrier plus her Human DNA / Face DNA.

## Runtime contract

- renderer loads each registered carrier exactly once;
- profiles resolve to a carrier through `CharacterProfileDefinition`;
- profile-specific eye runtime state may remain independent because it is dynamic state, not geometry residency;
- shaders receive profile identity/DNA while geometry buffers come from the resolved carrier;
- adding another female-derived profile must not create another female body/eye carrier allocation.

## Storage contract

Canonical prototype packages contain:

- two body carrier RMS0 assets;
- two eye carrier RMS0 assets;
- N compact profile genomes;
- one shared shader set;
- zero profile-specific duplicated carrier bytes.

## Future

The registry is the authority for carrier ownership. Later passes may make profiles data-driven and lazy-load carriers, but renderer code must never infer a carrier from a profile number.

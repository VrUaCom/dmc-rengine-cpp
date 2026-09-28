# DMC3 internal class graph

Generated from the SHA-256-bound executable. Each row is a class node; direct bases are outgoing inheritance links. Multiple inheritance is retained. These are recovered runtime types, not original source folders or complete class definitions. Decorated template/namespace names remain exact.

| Class | Type descriptor VA | Direct bases |
| --- | --- | --- |
| `.?AVFullMotionVideo@DMC3@@` | `0x1405d5118` | No base recorded in validated RTTI |
| `.?AVFullMotionVideoManager@DMC3@@` | `0x1405d5148` | No base recorded in validated RTTI |
| `.?AVRequest@AsyncIOThread@?A0xdf511c96@@` | `0x1405d5180` | No base recorded in validated RTTI |
| `.?AVReadRequest@AsyncIOThread@?A0xdf511c96@@` | `0x1405d51c0` | `.?AVRequest@AsyncIOThread@?A0xdf511c96@@` |
| `gfxClut` | `0x1405d5200` | `gfxTexture` |
| `gfxTexture` | `0x1405d5220` | No base recorded in validated RTTI |
| `IUnknown` | `0x1405d5248` | No base recorded in validated RTTI |
| `.?AVPCFullMotionVideoManager@DMC3@@` | `0x1405d5268` | `.?AVFullMotionVideoManager@DMC3@@` |
| `.?AUIXAudio2VoiceCallback@DMC3@@` | `0x1405d52a0` | No base recorded in validated RTTI |
| `.?AUIMMNotificationClient@DMC3@@` | `0x1405d52d8` | `IUnknown` |
| `.?AUPlaySoundStreamVoiceContext@DMC3@@` | `0x1405d5310` | `.?AUIXAudio2VoiceCallback@DMC3@@` |
| `.?AVPCFullMotionVideo@DMC3@@` | `0x1405d5348` | `.?AVFullMotionVideo@DMC3@@`, `.?AUIMMNotificationClient@DMC3@@` |
| `PCRenderTarget` | `0x1405d5378` | `gfxTexture` |
| `nbCriticalSection` | `0x1405d53a0` | No base recorded in validated RTTI |
| `PCCriticalSection` | `0x1405d53c8` | `nbCriticalSection` |
| `IActor` | `0x1405d53f0` | No base recorded in validated RTTI |
| `CWork` | `0x1405d5410` | No base recorded in validated RTTI |
| `CActor` | `0x1405d5430` | `IActor`, `CWork`, `ICollisionHandle` |
| `ICollisionHandle` | `0x1405d5450` | No base recorded in validated RTTI |
| `CAfterImage` | `0x1405d5478` | `CWork` |
| `ISceneFactory` | `0x1405d54a0` | No base recorded in validated RTTI |
| `CSceneMgr` | `0x1405d54c8` | No base recorded in validated RTTI |
| `CSceneFactoryApp` | `0x1405d54e8` | `ISceneFactory` |
| `CSceneMgrRoot` | `0x1405d5510` | `CSceneMgr` |
| `CSystemData` | `0x1405d5538` | No base recorded in validated RTTI |
| `CSpline` | `0x1405d5560` | No base recorded in validated RTTI |
| `CCameraRailBase` | `0x1405d5580` | `CCamera` |
| `CCamera` | `0x1405d55a8` | No base recorded in validated RTTI |
| `CCameraPan` | `0x1405d55c8` | `CCameraRailBase` |
| `CCameraRail` | `0x1405d55f0` | `CCameraRailBase` |
| `CCameraRailW` | `0x1405d5618` | `CCameraRailBase` |
| `CCameraRel` | `0x1405d5640` | `CCameraRailBase` |
| `ICom` | `0x1405d5668` | No base recorded in validated RTTI |
| `CCom` | `0x1405d5688` | `ICom` |
| `IComActionState` | `0x1405d56a8` | No base recorded in validated RTTI |
| `IComAction` | `0x1405d56d0` | `IComActionState` |
| `CComAction` | `0x1405d56f8` | `IComAction` |
| `CComEm000` | `0x1405d5720` | `CCom`, `IComAction` |
| `CComEm005` | `0x1405d5740` | `CCom`, `IComAction` |
| `CComEm006` | `0x1405d5760` | `CCom`, `IComAction` |
| `CComEm008` | `0x1405d5780` | `CCom`, `IComAction` |
| `CCustomize` | `0x1405d57a0` | No base recorded in validated RTTI |
| `CCustomizeData` | `0x1405d57c8` | No base recorded in validated RTTI |
| `IDamage` | `0x1405d57f0` | No base recorded in validated RTTI |
| `CDamage` | `0x1405d5810` | `ICollisionHandle`, `IDamage` |
| `CDamageDeath` | `0x1405d5830` | `CDamage` |
| `CDamageMulti` | `0x1405d5858` | `CDamage` |
| `CDamageIndex` | `0x1405d5880` | `CDamage` |
| `IDamageCalc` | `0x1405d58a8` | No base recorded in validated RTTI |
| `CDamageCalc` | `0x1405d58d0` | `IDamageCalc` |
| `CDemoProg` | `0x1405d58f8` | `CWork` |
| `IDifficult` | `0x1405d5918` | No base recorded in validated RTTI |
| `CDifficult` | `0x1405d5940` | `IDifficult` |
| `IDrawOperate` | `0x1405d5968` | No base recorded in validated RTTI |
| `CDrawOperate` | `0x1405d5990` | `IDrawOperate` |
| `IDraw` | `0x1405d59b8` | No base recorded in validated RTTI |
| `CDraw` | `0x1405d59d8` | `CDrawOperate`, `IDraw` |
| `CDrawSCM` | `0x1405d59f8` | `CDrawOperate`, `IDraw` |
| `IDrawCrush` | `0x1405d5a18` | No base recorded in validated RTTI |
| `CDrawCrush` | `0x1405d5a40` | `IDrawCrush` |
| `IDrawShadow` | `0x1405d5a68` | No base recorded in validated RTTI |
| `CDrawShadow` | `0x1405d5a90` | `IDrawShadow` |
| `IDrawUV` | `0x1405d5ab8` | No base recorded in validated RTTI |
| `CDrawUV` | `0x1405d5ad8` | `IDrawUV` |
| `CEfc` | `0x1405d5af8` | `CWork` |
| `CEfcCartridge` | `0x1405d5b18` | `CEfc` |
| `CEfcPub` | `0x1405d5b40` | `CEfc` |
| `CEfcShadow` | `0x1405d5b60` | `CEfc` |
| `CEm000` | `0x1405d5b88` | `CNonPlayerDeath` |
| `CNonPlayerDeath` | `0x1405d5ba8` | `CNonPlayer`, `INonPlayerDeath` |
| `CNonPlayer` | `0x1405d5bd0` | `CActor`, `INonPlayer` |
| `INonPlayer` | `0x1405d5bf8` | No base recorded in validated RTTI |
| `INonPlayerDeath` | `0x1405d5c20` | No base recorded in validated RTTI |
| `CEm000Shl00` | `0x1405d5c48` | `CShell` |
| `CShell` | `0x1405d5c70` | `CActor` |
| `CEm000Shl01` | `0x1405d5c90` | `CShell` |
| `CEm001` | `0x1405d5cb8` | `CNonPlayerDeath` |
| `CEm002` | `0x1405d5cd8` | `CNonPlayerDeath` |
| `CEm003` | `0x1405d5cf8` | `CNonPlayerDeath` |
| `CEm004` | `0x1405d5d18` | `CNonPlayerDeath` |
| `CEm005` | `0x1405d5d38` | `CNonPlayerDeath` |
| `CEm005Shl00` | `0x1405d5d58` | `CShell` |
| `CConstraint` | `0x1405d5d80` | No base recorded in validated RTTI |
| `CCnsChain` | `0x1405d5da8` | `CConstraint` |
| `CEm005Shl01` | `0x1405d5dc8` | `CNonPlayer` |
| `CEm006` | `0x1405d5df0` | `CNonPlayerDeath` |
| `CEm006Shl00` | `0x1405d5e10` | `CShell` |
| `CEm007` | `0x1405d5e38` | `CNonPlayerDeath` |
| `CCnsDirPos` | `0x1405d5e58` | `CConstraint` |
| `CEm008` | `0x1405d5e80` | `CNonPlayerDeath` |
| `CCnsRandom` | `0x1405d5ea0` | `CConstraint` |
| `CCnsIK` | `0x1405d5ec8` | `CConstraint` |
| `CCnsMatrix` | `0x1405d5ee8` | `CConstraint` |
| `CEm010` | `0x1405d5f10` | `CCom`, `IComAction`, `CNonPlayer` |
| `CEm010Baby` | `0x1405d5f30` | `CNonPlayer` |
| `CEm010Shl00` | `0x1405d5f58` | `CShell` |
| `CEm010Shl01` | `0x1405d5f80` | `CShell` |
| `CEm011` | `0x1405d5fa8` | `CCom`, `IComAction`, `CNonPlayer` |
| `CEm011Shl00` | `0x1405d5fc8` | `CShell` |
| `CSpreadBone` | `0x1405d5ff0` | No base recorded in validated RTTI |
| `CEm012` | `0x1405d6018` | `CCom`, `IComAction`, `CNonPlayer` |
| `CEm013` | `0x1405d6038` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm014` | `0x1405d6058` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm014Shl00` | `0x1405d6078` | `CShell` |
| `CEm014Shl01` | `0x1405d60a0` | `CShell` |
| `CEm016` | `0x1405d60c8` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm017` | `0x1405d60e8` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm017Master` | `0x1405d6108` | `CNonPlayer` |
| `CEm017Shl00` | `0x1405d6130` | `CShell` |
| `CEm019Shl00` | `0x1405d6158` | `CShell` |
| `CEm020Shl00` | `0x1405d6180` | `CShell` |
| `CEm021` | `0x1405d61a8` | `CEm017` |
| `CEm022Shl00` | `0x1405d61c8` | `CShell` |
| `CChainNode` | `0x1405d61f0` | No base recorded in validated RTTI |
| `CLight` | `0x1405d6218` | `CChainNode` |
| `CEm023Body` | `0x1405d6238` | `CCom`, `IComAction`, `CNonPlayer` |
| `CDamageEm023` | `0x1405d6260` | `CDamage` |
| `CEm023` | `0x1405d6288` | `CCom`, `IComAction`, `CNonPlayer` |
| `CEm023Shl00` | `0x1405d62a8` | `CShell` |
| `CEm023Shl01` | `0x1405d62d0` | `CShell` |
| `CEm023Shl02` | `0x1405d62f8` | `CShell` |
| `CEm023Shl03` | `0x1405d6320` | `CShell` |
| `CEm023Shl04` | `0x1405d6348` | `CShell` |
| `CEm023Shl05` | `0x1405d6370` | `CShell` |
| `CEm025` | `0x1405d6398` | `CCom`, `IComAction`, `CNonPlayer` |
| `CEm025Shl00` | `0x1405d63b8` | `CShell` |
| `CEm025Shl01` | `0x1405d63e0` | `CShell` |
| `CEm025Shl02` | `0x1405d6408` | `CShell` |
| `CEm026` | `0x1405d6430` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm026Shl00` | `0x1405d6450` | `CNonPlayer` |
| `CEm026Shl01` | `0x1405d6478` | `CNonPlayer` |
| `CEm027` | `0x1405d64a0` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm028` | `0x1405d64c0` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm028Shl00` | `0x1405d64e0` | `CShell2` |
| `CShell2` | `0x1405d6508` | `CNonPlayer` |
| `CEm028Shl01` | `0x1405d6528` | `CShell` |
| `CEm028Shl02` | `0x1405d6550` | `CShell` |
| `CEm028Shl03` | `0x1405d6578` | `CShell` |
| `CEm028Shl04` | `0x1405d65a0` | `CShell` |
| `CEm028Shl05` | `0x1405d65c8` | `CShell` |
| `CEm028Shl06` | `0x1405d65f0` | `CShell` |
| `CEm028Shl07` | `0x1405d6618` | `CShell` |
| `CEm029` | `0x1405d6640` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm029Cart` | `0x1405d6660` | `CEm029` |
| `CEm029Shl00` | `0x1405d6688` | `CShell` |
| `CEm029Shl01` | `0x1405d66b0` | `CShell` |
| `CEm029Shl02` | `0x1405d66d8` | `CShell` |
| `CEm029Shl03` | `0x1405d6700` | `CShell` |
| `CEm029Shl04` | `0x1405d6728` | `CShell` |
| `CEm029Shl05` | `0x1405d6750` | `CShell` |
| `CEm030` | `0x1405d6778` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm030Shl00` | `0x1405d6798` | `CShell` |
| `CEm030Shl01` | `0x1405d67c0` | `CShell` |
| `CEm031` | `0x1405d67e8` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm031Shl00` | `0x1405d6808` | `CNonPlayer` |
| `CDamageHaine2` | `0x1405d6830` | `CDamage` |
| `CEm032` | `0x1405d6858` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm032Shl00` | `0x1405d6878` | `CShell` |
| `CEm032Shl01` | `0x1405d68a0` | `CShell` |
| `CEm033` | `0x1405d68c8` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm034` | `0x1405d68e8` | `CCom`, `IComAction`, `CNonPlayer` |
| `CEm034Shl00` | `0x1405d6908` | `CShell` |
| `CEm034Shl01` | `0x1405d6930` | `CShell` |
| `CEm034Shl02` | `0x1405d6958` | `CShell` |
| `CEm034Shl03` | `0x1405d6980` | `CShell` |
| `CEm034Shl04` | `0x1405d69a8` | `CShell` |
| `CEm034Shl05` | `0x1405d69d0` | `CShell` |
| `CEm035` | `0x1405d69f8` | `CCom`, `IComAction`, `CNonPlayer` |
| `CEm035Shl00` | `0x1405d6a18` | `CShell` |
| `CEm035Shl01` | `0x1405d6a40` | `CShell` |
| `CEm035Shl02` | `0x1405d6a68` | `CShell` |
| `CEm037` | `0x1405d6a90` | `CCom`, `CComAction`, `CNonPlayer` |
| `CEm037Ball` | `0x1405d6ab0` | `CCom`, `IComAction`, `CNonPlayer` |
| `CEm037Shl00` | `0x1405d6ad8` | `CShell` |
| `CEm037Shl00Ctrl` | `0x1405d6b00` | `CShell` |
| `CEm037Shl01` | `0x1405d6b28` | `CShell` |
| `CEm037Shl01Ctrl` | `0x1405d6b50` | `CShell` |
| `CEm037Shl02` | `0x1405d6b78` | `CShell` |
| `CEm037Shl02Ctrl` | `0x1405d6ba0` | `CShell` |
| `CEm037Shl03` | `0x1405d6bc8` | `CShell` |
| `CEm037Shl04` | `0x1405d6bf0` | `CShell` |
| `CEm037Shl04Ctrl` | `0x1405d6c18` | `CShell` |
| `CEm037Shl05` | `0x1405d6c40` | `CShell` |
| `CEm037Shl05Ctrl` | `0x1405d6c68` | `CShell` |
| `CEm037Shl07` | `0x1405d6c90` | `CShell` |
| `CEm037ShlEffCtrl` | `0x1405d6cb8` | `CShell` |
| `CEmShl000` | `0x1405d6ce0` | `CShell` |
| `CEmShl01` | `0x1405d6d00` | `CShell` |
| `CStageSet` | `0x1405d6d20` | No base recorded in validated RTTI |
| `.?AV?$CList@VCNonPlayer@@@@` | `0x1405d6d40` | No base recorded in validated RTTI |
| `IFactoryEnemy` | `0x1405d6d70` | No base recorded in validated RTTI |
| `CFactoryEnemy` | `0x1405d6d98` | `IFactoryEnemy` |
| `CHeartShl00` | `0x1405d6dc0` | `CShell` |
| `CHeartShl01` | `0x1405d6de8` | `CShell` |
| `CHeartShl02` | `0x1405d6e10` | `CWork` |
| `IHitMark` | `0x1405d6e38` | No base recorded in validated RTTI |
| `CHitMark` | `0x1405d6e58` | `IHitMark` |
| `CItem` | `0x1405d6e78` | `CWork` |
| `CItemGet` | `0x1405d6e98` | No base recorded in validated RTTI |
| `CItemKey` | `0x1405d6eb8` | `CItem` |
| `CItemOrb` | `0x1405d6ed8` | `CItem` |
| `CMisSelect` | `0x1405d6ef8` | No base recorded in validated RTTI |
| `CNonPlayerDeathEffect` | `0x1405d6f20` | `CWork` |
| `CNonPlayerDeathEffect2` | `0x1405d6f50` | `CWork` |
| `IPermitAttack` | `0x1405d6f80` | No base recorded in validated RTTI |
| `CPermitAttack` | `0x1405d6fa8` | `IPermitAttack` |
| `CPl000Shl00` | `0x1405d6fd0` | `CShell` |
| `CPl000Shl01` | `0x1405d6ff8` | `CShell` |
| `CPl000Shl02` | `0x1405d7020` | `CShell` |
| `CPl000Shl03` | `0x1405d7048` | `CShell` |
| `CPl000Shl04` | `0x1405d7070` | `CShell` |
| `CPl000Shl05` | `0x1405d7098` | `CShell` |
| `CPl000Shl06` | `0x1405d70c0` | `CShell` |
| `CPl000Shl07` | `0x1405d70e8` | `CShell` |
| `CPl000Shl08` | `0x1405d7110` | `CShell` |
| `CPl000Shl09` | `0x1405d7138` | `CShell` |
| `CPl000Shl0a` | `0x1405d7160` | `CShell` |
| `CPl000Shl0b` | `0x1405d7188` | `CShell` |
| `CPl000Shl0c` | `0x1405d71b0` | `CShell` |
| `CPl000Shl0cICE` | `0x1405d71d8` | `CShell` |
| `CPl000Shl0d` | `0x1405d7200` | `CShell` |
| `CPl000Shl0e` | `0x1405d7228` | `CShell` |
| `CPl000Shl0f` | `0x1405d7250` | `CShell` |
| `CPl000Shl10` | `0x1405d7278` | `CShell` |
| `CPl021Shl00` | `0x1405d72a0` | `CShell` |
| `CPl021Shl01` | `0x1405d72c8` | `CShell` |
| `CPl021Shl02` | `0x1405d72f0` | `CShell` |
| `CPl021Shl03` | `0x1405d7318` | `CShell` |
| `.?AV?$CList@VCLockOnTarget@@@@` | `0x1405d7340` | No base recorded in validated RTTI |
| `CKeyBuffer` | `0x1405d7370` | No base recorded in validated RTTI |
| `CPlDante` | `0x1405d7398` | `CPlayer` |
| `CPlayer` | `0x1405d73b8` | `CActor`, `IPlayer` |
| `IPlayer` | `0x1405d73d8` | No base recorded in validated RTTI |
| `CPlVergil` | `0x1405d73f8` | `CPlayer` |
| `CPlLady` | `0x1405d7418` | `CPlayer` |
| `CPlNewVergil` | `0x1405d7438` | `CPlayer` |
| `CPlayerWeapon` | `0x1405d7460` | `CActor` |
| `CPlWp2Sword` | `0x1405d7488` | `CPlayerWeapon` |
| `CPlWpFight` | `0x1405d74b0` | `CPlayerWeapon` |
| `CPlWpFoeceEdge` | `0x1405d74d8` | `CPlayerWeapon` |
| `CPlWpGuitar` | `0x1405d7500` | `CPlayerWeapon` |
| `CString` | `0x1405d7528` | `CWork` |
| `CPlWpGun` | `0x1405d7548` | `CPlayerWeapon` |
| `CPlWpLadyGun` | `0x1405d7568` | `CPlayerWeapon` |
| `CPlWpLaser` | `0x1405d7590` | `CPlayerWeapon` |
| `CPlWpNeroSword` | `0x1405d75b8` | `CPlayerWeapon` |
| `CPlWpNewVergilSword` | `0x1405d75e0` | `CPlayerWeapon` |
| `CPlWpNunchaku` | `0x1405d7610` | `CPlayerWeapon` |
| `CPlWpRifle` | `0x1405d7638` | `CPlayerWeapon` |
| `CPlWpShotGun` | `0x1405d7660` | `CPlayerWeapon` |
| `CPlWpSword` | `0x1405d7688` | `CPlayerWeapon` |
| `CPlWpVergilSword` | `0x1405d76b0` | `CPlayerWeapon` |
| `CEffectBase` | `0x1405d76d8` | `CWork` |
| `CParticle` | `0x1405d7700` | `CEffectBase` |
| `CRealParticle` | `0x1405d7720` | `CParticle` |
| `CFakeParticle` | `0x1405d7748` | `CParticle` |
| `CPtclLine00` | `0x1405d7770` | `CRealParticle` |
| `CFPtclLine00` | `0x1405d7798` | `CFakeParticle` |
| `CPtclLine01` | `0x1405d77c0` | `CRealParticle` |
| `CFPtclLine01` | `0x1405d77e8` | `CFakeParticle` |
| `CPtclLine02` | `0x1405d7810` | `CRealParticle` |
| `CFPtclLine02` | `0x1405d7838` | `CFakeParticle` |
| `CPtclPoly00` | `0x1405d7860` | `CRealParticle` |
| `CFPtclPoly00` | `0x1405d7888` | `CFakeParticle` |
| `CPtclPoly01` | `0x1405d78b0` | `CRealParticle` |
| `CFPtclPoly01` | `0x1405d78d8` | `CFakeParticle` |
| `CPtclSprt00` | `0x1405d7900` | `CRealParticle` |
| `CFPtclSprt00` | `0x1405d7928` | `CFakeParticle` |
| `CScene` | `0x1405d7950` | No base recorded in validated RTTI |
| `CSceneMgrGame` | `0x1405d7970` | `CSceneMgr` |
| `CChain` | `0x1405d7998` | No base recorded in validated RTTI |
| `CLightMgr` | `0x1405d79b8` | No base recorded in validated RTTI |
| `CLightStatic` | `0x1405d79d8` | No base recorded in validated RTTI |
| `SLoadOneGame` | `0x1405d7a00` | `CChainNode` |
| `CGameLoader` | `0x1405d7a28` | No base recorded in validated RTTI |
| `CSceneBoot` | `0x1405d7a50` | `CScene` |
| `CSceneOpening` | `0x1405d7a78` | `CScene` |
| `CSceneStartMenu` | `0x1405d7aa0` | `CScene` |
| `CEventMission` | `0x1405d7ac8` | `CEventMissionLight` |
| `CEventMissionLight` | `0x1405d7af0` | No base recorded in validated RTTI |
| `CGameW` | `0x1405d7b20` | No base recorded in validated RTTI |
| `CSceneMisSelect` | `0x1405d7b40` | `CScene` |
| `CSceneGameMain` | `0x1405d7b68` | `CScene` |
| `CSceneGame` | `0x1405d7b90` | `CScene` |
| `CCameraPlayer` | `0x1405d7bb8` | `CCamera` |
| `CCameraBoss` | `0x1405d7be0` | `CCameraPlayer` |
| `CCameraCtrl` | `0x1405d7c08` | `CCamera` |
| `CCameraMiniDemo` | `0x1405d7c30` | `CCamera` |
| `CSceneDemo` | `0x1405d7c58` | `CScene` |
| `CSceneMisStart` | `0x1405d7c80` | `CScene` |
| `CExtraInfo` | `0x1405d7ca8` | No base recorded in validated RTTI |
| `CSceneResult` | `0x1405d7cd0` | `CScene` |
| `CSceneEnding` | `0x1405d7cf8` | `CScene` |
| `CDrawBreak` | `0x1405d7d20` | `CDraw` |
| `CScrnBreak` | `0x1405d7d48` | `CWork` |
| `CStaffRoll` | `0x1405d7d70` | `CWork` |
| `CStage` | `0x1405d7d98` | `CWork` |
| `CStageSetBingo` | `0x1405d7db8` | `CActor`, `CStageSet` |
| `CStageSetBreak` | `0x1405d7de0` | `CActor`, `CStageSet` |
| `CStageSetShl00` | `0x1405d7e08` | `CActor` |
| `CStageSetBrocken` | `0x1405d7e30` | `CActor`, `CStageSet` |
| `CStageSetCatapult` | `0x1405d7e58` | `CActor`, `CStageSet` |
| `CStageSetCollision` | `0x1405d7e80` | `CActor`, `CStageSet` |
| `CStageSetColor` | `0x1405d7eb0` | `CWork`, `CStageSet` |
| `CStageSetCombo` | `0x1405d7ed8` | `CActor`, `CStageSet` |
| `CStageSetCrystal` | `0x1405d7f00` | `CActor`, `CStageSet` |
| `CStageSetEye` | `0x1405d7f28` | `CActor`, `CStageSet` |
| `CStageSetFence` | `0x1405d7f50` | `CActor`, `CStageSet` |
| `CStageSetGate` | `0x1405d7f78` | `CActor`, `CStageSet` |
| `CStageSetGearArch` | `0x1405d7fa0` | `CActor`, `CStageSet` |
| `CStageSetHang` | `0x1405d7fc8` | `CActor`, `CStageSet` |
| `CStageSetHang2` | `0x1405d7ff0` | `CActor`, `CStageSet` |
| `CStageSetHeart` | `0x1405d8018` | `CNonPlayer`, `CStageSet` |
| `CStageSetLift` | `0x1405d8040` | `CActor`, `CStageSet` |
| `CStageSetLight` | `0x1405d8068` | `CWork`, `CStageSet` |
| `CStageSetLungs` | `0x1405d8090` | `CNonPlayer`, `CStageSet` |
| `CStageSetMirror` | `0x1405d80b8` | `CActor`, `CStageSet` |
| `CStageSetNausika` | `0x1405d80e0` | `CNonPlayer`, `CStageSet` |
| `CStageSetNeedle` | `0x1405d8108` | `CActor`, `CStageSet` |
| `CStageSetORBreak` | `0x1405d8130` | `CActor`, `CStageSet` |
| `CStageSetRoad` | `0x1405d8158` | `CActor`, `CStageSet` |
| `CStageSetSe` | `0x1405d8180` | `CWork`, `CStageSet` |
| `CStageSetSeal` | `0x1405d81a8` | `CActor`, `CStageSet` |
| `CStageSetSlide` | `0x1405d81d0` | `CActor`, `CStageSet` |
| `CStageSetStayDemo` | `0x1405d81f8` | `CActor`, `CStageSet` |
| `CStageSetStay` | `0x1405d8220` | `CStageSetStayDemo` |
| `CStageSetStomach` | `0x1405d8248` | `CActor`, `CStageSet` |
| `CStageSetSwitch` | `0x1405d8270` | `CActor`, `CStageSet` |
| `CStageSetTruck` | `0x1405d8298` | `CActor`, `CStageSet` |
| `CStageSetTruckEnemy` | `0x1405d82c0` | `CActor` |
| `CStageSetYure` | `0x1405d82f0` | `CWork`, `CStageSet` |
| `CStatus` | `0x1405d8318` | No base recorded in validated RTTI |
| `CUIDBright` | `0x1405d8338` | `CWork` |
| `CUIDCockpit00` | `0x1405d8360` | `CWork` |
| `CUIDCockpit01` | `0x1405d8388` | `CWork` |
| `CUIDCockpit02` | `0x1405d83b0` | `CWork` |
| `CUIDComnBg` | `0x1405d83d8` | `CWork` |
| `CUIDControl` | `0x1405d8400` | `CWork` |
| `CUIDCustomGun` | `0x1405d8428` | `CWork` |
| `CUIDCustomGunV` | `0x1405d8450` | `CWork` |
| `CUIDCustomIndex` | `0x1405d8478` | `CWork` |
| `CUIDCustomItem` | `0x1405d84a0` | `CWork` |
| `CUIDCustomSkill` | `0x1405d84c8` | `CWork` |
| `CUIDCustomWeapon` | `0x1405d84f0` | `CWork` |
| `CUIDDemoDigest` | `0x1405d8518` | `CWork` |
| `CUIDExtraInfo` | `0x1405d8540` | `CWork` |
| `CUIDGallery` | `0x1405d8568` | `CWork` |
| `CUIDGameOptions` | `0x1405d8590` | `CWork` |
| `CUIDGoldOrb` | `0x1405d85b8` | `CWork` |
| `CUIDItemGet` | `0x1405d85e0` | `CWork` |
| `CUIDLanguage` | `0x1405d8608` | `CWork` |
| `CUIDLockOnCursor` | `0x1405d8630` | `CWork` |
| `CUIDMemoryCard` | `0x1405d8658` | `CWork` |
| `CUIDMisSelect` | `0x1405d8680` | `CWork` |
| `CUIDMissionStart` | `0x1405d86a8` | `CWork` |
| `CUIDOption` | `0x1405d86d0` | `CWork` |
| `CUIDPause` | `0x1405d86f8` | `CWork` |
| `CUIDResult` | `0x1405d8718` | `CWork` |
| `CUIDResultBP` | `0x1405d8740` | `CWork` |
| `CUIDSaveBg` | `0x1405d8768` | `CWork` |
| `CUIDScreen` | `0x1405d8790` | `CWork` |
| `CUIDSmorkBg` | `0x1405d87b8` | `CWork` |
| `CUIDStaffRoll` | `0x1405d87e0` | `CWork` |
| `CUIDStatusFile` | `0x1405d8808` | `CWork` |
| `CUIDStatusFileIpu` | `0x1405d8830` | `CWork` |
| `CUIDStatusFileSec` | `0x1405d8858` | `CWork` |
| `CUIDStatusIndex` | `0x1405d8880` | `CWork` |
| `CUIDStatusItem` | `0x1405d88a8` | `CWork` |
| `CUIDStatusMap` | `0x1405d88d0` | `CWork` |
| `CUIDStatusWeapon` | `0x1405d88f8` | `CWork` |
| `CUIDStyleLvup` | `0x1405d8920` | `CWork` |
| `CUIDStyleSelect` | `0x1405d8948` | `CWork` |
| `CUIDStyleSelectV` | `0x1405d8970` | `CWork` |
| `CUIDStylishCombo` | `0x1405d8998` | `CWork` |
| `CUIDTitle` | `0x1405d89c0` | `CWork` |
| `CUIDTotalRanking` | `0x1405d89e0` | `CWork` |
| `CUIDTutorial` | `0x1405d8a08` | `CWork` |
| `CUIDVergilWeapon` | `0x1405d8a30` | `CWork` |
| `CCameraMotion` | `0x1405d8a58` | `CCamera` |
| `CDemoModelWork` | `0x1405d8a80` | `CWork` |
| `CEffect` | `0x1405d8aa8` | `CEffectBase` |
| `IFadeControl` | `0x1405d8ac8` | No base recorded in validated RTTI |
| `CFadeControl` | `0x1405d8af0` | `IFadeControl` |
| `CGenerator` | `0x1405d8b18` | `CEffectBase` |
| `CMotion` | `0x1405d8b40` | `CDrawOperate` |
| `IPtxManager` | `0x1405d8b60` | No base recorded in validated RTTI |
| `CPtxManager` | `0x1405d8b88` | `IPtxManager` |
| `CClip` | `0x1405d8bb0` | No base recorded in validated RTTI |
| `CSoundClip` | `0x1405d8bd0` | `CClip` |
| `CWorkRateClip` | `0x1405d8bf8` | `CClip` |
| `CModelClip` | `0x1405d8c20` | `CClip` |
| `CMotionClip` | `0x1405d8c48` | `CClip` |
| `CHideClip` | `0x1405d8c70` | `CClip` |
| `CProgramClip` | `0x1405d8c90` | `CClip` |
| `CMessageClip` | `0x1405d8cb8` | `CClip` |
| `CPadVibeClip` | `0x1405d8ce0` | `CClip` |
| `CEffectClip` | `0x1405d8d08` | `CClip` |
| `CCameraClip` | `0x1405d8d30` | `CClip` |
| `CLightClip` | `0x1405d8d58` | `CClip` |
| `CScrEfcClip` | `0x1405d8d80` | `CClip` |
| `CFadeClip` | `0x1405d8da8` | `CClip` |
| `CValue` | `0x1405d8dc8` | `CEffectBase` |
| `CMcAppli` | `0x1405d8de8` | No base recorded in validated RTTI |
| `type_info` | `0x1405d8e08` | No base recorded in validated RTTI |
| `.?AVbad_alloc@std@@` | `0x1405d8e28` | `.?AVexception@std@@` |
| `.?AVexception@std@@` | `0x1405d8e50` | No base recorded in validated RTTI |
| `.?AVbad_array_new_length@std@@` | `0x1405d8e78` | `.?AVbad_alloc@std@@` |

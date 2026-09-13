plugins {
    id("com.android.application")
}

android {
    namespace = "com.vruacom.dmcrengine.runtime"
    compileSdk = 36
    ndkVersion = "27.0.12077973"

    defaultConfig {
        applicationId = "com.vruacom.dmcrengine.runtime"
        minSdk = 26
        targetSdk = 36
        versionCode = 1
        versionName = "0.1.0-runtime-foundation"

        ndk {
            // 64-bit only. The runtime has no 32-bit validation story and will
            // not ship an ABI it does not test.
            abiFilters += listOf("arm64-v8a", "x86_64")
        }

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DDMC_RENGINE_RUNTIME_CXX_STANDARD=23",
                )
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("../CMakeLists.txt")
            version = "3.31.6"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    buildTypes {
        release {
            isMinifyEnabled = false
        }
    }
}

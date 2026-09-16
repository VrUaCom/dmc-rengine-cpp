plugins { id("com.android.application") }

android {
    namespace = "com.vruacom.rengine.lsg"
    compileSdk = 36
    ndkVersion = "30.0.16248370"

    defaultConfig {
        applicationId = "com.vruacom.rengine.lsg"
        minSdk = 26
        targetSdk = 36
        versionCode = 1
        versionName = "0.1.0"
        ndk { abiFilters += "arm64-v8a" }
        externalNativeBuild {
            cmake {
                arguments += listOf("-DRENGINE_LSG_BUILD_TESTS=OFF", "-DRENGINE_LSG_BUILD_VIEWER=OFF")
                cppFlags += listOf("-std=c++23", "-Wall", "-Wextra")
            }
        }
    }
    externalNativeBuild {
        cmake {
            path = file("../../../CMakeLists.txt")
            version = "3.22.1"
        }
    }
    packaging { jniLibs { useLegacyPackaging = false } }
}

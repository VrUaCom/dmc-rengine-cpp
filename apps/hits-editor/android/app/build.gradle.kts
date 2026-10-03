plugins { id("com.android.application") }
android {
    namespace = "com.dmcrengine.hitseditor"
    compileSdk = 36
    ndkVersion = "30.0.16248370"
    defaultConfig {
        applicationId = "com.dmcrengine.hitseditor"
        minSdk = 26
        targetSdk = 36
        versionCode = 1
        versionName = "0.1.0-preview"
        ndk { abiFilters += listOf("arm64-v8a") }
    }
    externalNativeBuild {
        cmake { path = file("src/main/cpp/CMakeLists.txt"); version = "3.22.1" }
    }
    buildTypes {
        debug { isJniDebuggable = false }
        release { isMinifyEnabled = false }
    }
}

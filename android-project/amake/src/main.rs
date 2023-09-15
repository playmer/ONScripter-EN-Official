use std::{process::Command, path::{Path, PathBuf}, collections::HashMap, io::Write, fs::OpenOptions};
use walkdir::WalkDir;
use zip_next::write::FileOptions;

struct Configuration
{
    cmakeliststxt_dir : PathBuf,
    java_code_dir : PathBuf,
    android_tools_dir : PathBuf,
    android_build_dir : PathBuf,
    android_platform_path : PathBuf
}

impl Configuration
{
    fn new() -> Configuration
    {
        let android_ndk_home = std::env::var("ANDROID_NDK_HOME").expect("Couldn't retrieve env variable ANDROID_NDK_HOME, please set it");
        let android_platform_version = std::env::var("ANDROID_PLATFORM").expect("Couldn't retrieve env variable ANDROID_PLATFORM, please set it");
        let android_sdk_path = Path::new(&android_ndk_home).join("..").join("..");

        // Prefer to use the platform jar of the preferred version, otherwise use latest.
        let mut android_platform_path = android_sdk_path.join("platforms").join(format!("android-{android_platform_version}"));
        let metadata = std::fs::metadata(&android_platform_path);
        if metadata.is_err() || !metadata.unwrap().is_dir() {
            let mut android_platforms = Vec::new();

            for entry in std::fs::read_dir(&android_sdk_path.join("platforms")).unwrap() {
                let entry = entry.unwrap();
                let path = entry.path();
                let path = path.file_name().unwrap().to_str().unwrap();
                let version = path.split_at("android-".len()).1;

                android_platforms.push(version.parse::<i32>().unwrap());
            }
            android_platforms.sort();
            android_platform_path = android_sdk_path.join("platforms").join(format!("android-{}", android_platforms.last().unwrap()));
        }

        // Find the latest version of the android tools
        let mut android_tools_versions = Vec::new();
        
        for entry in std::fs::read_dir(&android_sdk_path.join("build-tools")).unwrap() {
            let entry = entry.unwrap();
            let path = entry.path();
            let metadata = std::fs::metadata(&path).unwrap();

            if metadata.is_dir() {
                let path = path.file_name().unwrap().to_str().unwrap();
                android_tools_versions.push(semver::Version::parse(path).unwrap());
            }
        }

        android_tools_versions.sort();
        let ver : &semver::Version = android_tools_versions.last().unwrap();
        let android_tools_dir = android_sdk_path.join("build-tools").join(format!("{}.{}.{}", ver.major, ver.minor, ver.patch));

        println!("{}", android_tools_dir.to_str().unwrap());
        
        return Configuration { 
            cmakeliststxt_dir: Path::new("../").to_owned(),
            android_tools_dir: android_tools_dir,
            java_code_dir: Path::new("src").to_owned(),
            android_build_dir: Path::new("build").to_owned(),
            android_platform_path: android_platform_path
        };
    }
}

fn run_cmake_configure_preset(config : &Configuration, presets : &Vec<(String, String)>) -> bool
{
    for (configure_preset, build_preset) in presets
    {
        let status = Command::new("cmake")
            .arg("--preset")
            .arg(&configure_preset)
            .arg(".")
            .current_dir(&config.cmakeliststxt_dir)
            .status()
            .expect("failed to execute cmake.");

        if status.code().is_none() || (status.code().unwrap() != 0)
        {
            println!("Failed to configure preset {configure_preset}");
            return false;
        }

        let status = Command::new("cmake")
            .arg("--build")
            .arg("--preset")
            .arg(&build_preset)
            .current_dir(&config.cmakeliststxt_dir)
            .status()
            .expect("failed to execute cmake.");

        if status.code().is_none() || (status.code().unwrap() != 0)
        {
            println!("Failed to configure preset {build_preset}");
            return false;;
        }
    }

    return true;
}

fn run_javac(config : &Configuration)
{
    let mut java_files = Vec::new();
    for entry in WalkDir::new(&config.java_code_dir)
            .into_iter()
            .filter_map(Result::ok)
            .filter(|e| !e.file_type().is_dir())
    {
        let entryPath = entry.path().to_owned();
        if entryPath.extension().is_some() && (entryPath.extension().unwrap() == "java")
        {
            println!("File {}", entryPath.display());
            java_files.push(entryPath);
        }
    }
    
    let mut binding = Command::new("javac");
    let mut binding  = binding
        .arg("-source")
        .arg("11")
        .arg("-target")
        .arg("11")
        .arg("-encoding")
        .arg("utf8")
        .arg("-classpath")
        .arg(config.android_platform_path.join("android.jar"))
        .arg("-d")
        .arg(&config.android_build_dir);

    for file in java_files
    {
        binding.arg(&file);
    }

    binding.status().expect("Failed to run javac");

}

fn run_d8(config : &Configuration)
{
    let mut class_files = Vec::new();
    for entry in WalkDir::new(&config.android_build_dir)
            .into_iter()
            .filter_map(Result::ok)
            .filter(|e| !e.file_type().is_dir())
    {
        let entryPath = entry.path().to_owned();
        if entryPath.extension().is_some() && (entryPath.extension().unwrap() == "class")
        {
            println!("File {}", entryPath.display());
            class_files.push(entryPath);
        }
    }
    
    let mut binding = Command::new(config.android_tools_dir.join("d8.bat"));
    let mut binding = binding
        .arg("--classpath")
        .arg(&config.android_platform_path.join("android.jar"))
        .arg("--output")
        .arg(&config.android_build_dir);

    for file in class_files
    {
        binding.arg(&file);
    }

    binding.status().expect("Failed to run d8");
}

fn make_apk(config : &Configuration, presets : &Vec<(String, String)>)
{
    let mut configToAbiMap : HashMap<&str, &str> = HashMap::new();
    configToAbiMap.insert("android_arm64", "arm64-v8a");
    configToAbiMap.insert("android_armNeon", "armeabi-v7a");
    configToAbiMap.insert("android_x86_64", "x86_64");
    configToAbiMap.insert("android_x86", "x86");
    configToAbiMap.insert("android_armv6", "armeabi"); // FIXME
    let configToAbiMap = configToAbiMap;

    let status = Command::new(config.android_tools_dir.join("aapt2"))
        .arg("compile")
        .arg("-o")
        .arg(config.android_build_dir.join("res.zip"))
        .arg("--dir")
        .arg("res")
        .status()
        .expect("Failed to run aapt2 to build resources file");




    for (configure_preset, _build_preset) in presets
    {
        let native_lib_dir = Path::new("build").join("native").join("lib").join(&configToAbiMap.get(configure_preset.as_str()).unwrap());
        let lib_file = native_lib_dir.join("libonscripter_en.so", );
        let lib_file_path = config.cmakeliststxt_dir.join("builds").join(configure_preset).join("lib").join("Release").join("libonscripter_en.so");
        std::fs::create_dir_all(native_lib_dir).unwrap();
        std::fs::copy(lib_file_path, lib_file).unwrap();
    }
    let status = Command::new(config.android_tools_dir.join("aapt2"))
        .arg("compile")
        .arg("-o")
        .arg(config.android_build_dir.join("native_resources.zip"))
        .arg("--dir")
        .arg("build/native")
        .status()
        .expect("Failed to run aapt2 to build resources file");





    let unaligned_path = config.android_build_dir.join("unaligned.apk");




    // Create the initial Apk
    let status = Command::new(config.android_tools_dir.join("aapt2"))
        .arg("link")
        .arg("-o")
        .arg(&unaligned_path)
        .arg("--manifest")
        .arg("AndroidManifest.xml")
        .arg("-I")
        .arg(config.android_platform_path.join("android.jar"))
        .arg("--emit-ids")
        .arg("ids.txt")
        .arg(config.android_build_dir.join("res.zip"))
        .status()
        .expect("Failed to run aapt2 to build initial APK");







    let status = Command::new("C:/repos/ONScripter-EN-Official/android-project/7za.exe")
        .current_dir(&config.android_build_dir) // 7zip is cursed and is placing the file in build/fileName without running from the build dir, but only for this.
        .arg("a")
        .arg("unaligned.apk")
        .arg("-tzip")
        .arg("classes.dex").status().expect("Failed to add to unaligned APK");
    
    if status.code().is_none() || (status.code().unwrap() != 0)
    {
        println!("Failed to embed classes.dex");
        return;
    }

    for (configure_preset, _build_preset) in presets
    {
        let native_lib_dir = Path::new("lib").join(&configToAbiMap.get(configure_preset.as_str()).unwrap());
        let lib_file = native_lib_dir.join("libonscripter_en.so", );
        let lib_file_path = config.cmakeliststxt_dir.join("builds").join(configure_preset).join("lib").join("Release").join("libonscripter_en.so");


        let status = Command::new("C:/repos/ONScripter-EN-Official/android-project/7za.exe")
            .arg("a")
            .arg(&unaligned_path)
            .arg("-tzip")
            .arg(&lib_file_path).status().expect("Failed to add to unaligned APK");
        
        if status.code().is_none() || (status.code().unwrap() != 0)
        {
            println!("Failed to embed {}", &native_lib_dir.display());
            return;
        }
        let status = Command::new("C:/repos/ONScripter-EN-Official/android-project/7za.exe")
            .arg("rn")
            .arg(&unaligned_path)
            .arg("-tzip")
            .arg("libonscripter_en.so")
            .arg(&native_lib_dir.join("libonscripter_en.so")).status().expect("Failed to rename to in unaligned APK");
        
        if status.code().is_none() || (status.code().unwrap() != 0)
        {
            println!("Failed to rename native lib to {}", &native_lib_dir.display());
            return;
        }
    }


    // FIXME: There's something suble not working here, where we write to the file but it doesn't seem to be added to the archive.
    // Open the unaligned apk file as a zip
//    {
//        let file = OpenOptions::new().read(true).append(true).open(&unaligned_path).unwrap();
//        let mut zip = zip_next::ZipWriter::new_append(file).unwrap();
//        zip.set_flush_on_finish_file(true);
//    
//        // Append classes file onto the apk
//        zip.start_file("classes.dex", FileOptions::default()).unwrap();
//        let classes_file = config.android_build_dir.join("classes.dex");
//        let file_data = std::fs::read(classes_file).unwrap();
//        zip.write_all(file_data.as_ref()).unwrap();
//        
//        zip.add_directory("lib", Default::default()).unwrap();
//    
//        // Append the native libraries onto the apk.
//        for (configure_preset, _build_preset) in presets
//        {
//            zip.add_directory(format!("lib/{}", configToAbiMap.get(configure_preset.as_str()).unwrap()), Default::default()).unwrap();
//            zip.start_file(format!("lib/{}/libonscripter_en.so", configToAbiMap.get(configure_preset.as_str()).unwrap()), FileOptions::default()).unwrap();
//            let lib_file = config.cmakeliststxt_dir.join("builds").join(configure_preset).join("lib").join("Release").join("libonscripter_en.so");
//            let file_data = std::fs::read(lib_file).unwrap();
//            zip.write_all(file_data.as_ref()).unwrap();
//        }
//        zip.finish().unwrap();
//    }

    println!("Calling Zipalign to produced the aligned APK before signing.");
    let status = Command::new(config.android_tools_dir.join("zipalign"))
        .arg("-f")
        .arg("4")
        .arg(&unaligned_path)
        .arg(&config.android_build_dir.join("aligned.apk"))
        .status()
        .expect("Failed to Aligned APK");
    
    if status.code().is_none() || (status.code().unwrap() != 0)
    {
        println!("Failed to align apk.");
        return;
    }
    

    

    println!("Sign the APK");
    let status = Command::new(config.android_tools_dir.join("apksigner.bat"))
        .arg("sign")
        .arg("--ks")
        .arg("$KEYSTORE")
        .arg("--ks-pass")
        .arg("\"pass:$KS_PASS\"")
        .arg("--min-sdk-version")
        .arg("15")
        .arg("--out")
        .arg("signed.apk")
        .arg(&config.android_build_dir.join("aligned.apk"))
        .status()
        .expect("Failed to sign APK");
    
    if status.code().is_none() || (status.code().unwrap() != 0)
    {
        println!("Failed to sign apk.");
        return;
    }

}

fn main()
{
    std::env::set_current_dir("../").unwrap();

    let config = Configuration::new();

    let mut presets : Vec<(String, String)> = Vec::new();
    presets.push(("android_x86_64".to_string(), "android-x86_64-release".to_string()));
    presets.push(("android_x86".to_string(), "android-x86-release".to_string()));
    presets.push(("android_arm64".to_string(), "android-arm64-release".to_string()));
    presets.push(("android_armNeon".to_string(), "android-armNeon-release".to_string()));
    //if !run_cmake_configure_preset(&config, &presets) {
    //    println!("Failed to run CMake");
    //    return;
    //}
    std::fs::remove_dir_all(&config.android_build_dir).unwrap_or_default();
    run_javac(&config);
    run_d8(&config);
    make_apk(&config, &presets);
}

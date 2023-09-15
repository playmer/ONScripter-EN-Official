use std::{process::Command, path::{Path, PathBuf}, collections::HashMap, io::Write};
use walkdir::WalkDir;
use zip::write::FileOptions;

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
            .expect("failed to execute StaticGraphCompilations");

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
            .expect("failed to execute StaticGraphCompilations");

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

    let mut status = Command::new(config.android_tools_dir.join("aapt2"))
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
    
    let mut status = Command::new(config.android_tools_dir.join("aapt2"))
        .arg("compile")
        .arg("-o")
        .arg(config.android_build_dir.join("native_resources.zip"))
        .arg("--dir")
        .arg("build/native")
        .status()
        .expect("Failed to run aapt2 to build resources file");










    // Create the initial Apk
    let mut status = Command::new(config.android_tools_dir.join("aapt2"))
        .arg("link")
        .arg("-o")
        .arg(config.android_build_dir.join("unaligned.apk"))
        .arg("--manifest")
        .arg("AndroidManifest.xml")
        .arg("-I")
        .arg(config.android_platform_path.join("android.jar"))
        .arg("--emit-ids")
        .arg("ids.txt")
        .arg(config.android_build_dir.join("res.zip"))
        .status()
        .expect("Failed to run aapt2 to build initial APK");

    //// Open the unaligned apk file as a zip
    //let file = std::fs::File::create(Path::new(&config.android_build_dir).join("unaligned.apk")).unwrap();
    //let mut zip = zip::ZipWriter::new_append(file).unwrap();
    //// Append classes file onto the apk
    //let options = FileOptions::default()
    //    .compression_method(zip::CompressionMethod::Stored)
    //    .compression_level(None);
    //zip.start_file_aligned("classes.dex", options, 4).unwrap();
    //let classes_file = config.android_build_dir.join("classes.dex");
    //let file_data = std::fs::read(classes_file).unwrap();
    //zip.write(file_data.as_ref()).unwrap();
    //// Append the native libraries onto the apk.
    //for (configure_preset, _build_preset) in presets
    //{
    //    let options = FileOptions::default()
    //        .compression_method(zip::CompressionMethod::Stored)
    //        .compression_level(None);
    //    zip.start_file_aligned(format!("lib/{}/libonscripter_en.so", configToAbiMap.get(configure_preset.as_str()).unwrap()), options, 4).unwrap();
    //    let lib_file = config.cmakeliststxt_dir.join("builds").join(configure_preset).join("lib").join("Release").join("libonscripter_en.so");
    //    let file_data = std::fs::read(lib_file).unwrap();
    //    zip.write(file_data.as_ref()).unwrap();
    //}
    //zip.finish().unwrap();
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
    if !run_cmake_configure_preset(&config, &presets) {
        println!("Failed to run CMake");
        return;
    }
    
    std::fs::remove_dir_all(&config.android_build_dir).unwrap_or_default();

    run_javac(&config);
    run_d8(&config);
    make_apk(&config, &presets);
}

use std::{process::Command, path::Path};
use walkdir::WalkDir;

struct Configuration
{
    cmakeliststxt_dir : String,
    tools_dir : String,
    java_code_dir : String,
    android_build_dir : String,
    android_jar_path : String
}

fn run_cmake_configure_preset(config : &Configuration, presets : &Vec<(String, String)>)
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
            continue;
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
            continue;
        }
    }
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
        .arg(&config.android_jar_path)
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
    
    let mut binding = Command::new(Path::new(&config.tools_dir).join("d8.bat"));
    let mut binding = binding
        .arg("--classpath")
        .arg(&config.android_jar_path)
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
    // Create the initial Apk
    // $TOOLS_DIR/aapt2 link -o build/unaligned.apk --manifest AndroidManifest.xml -I $PLATFORM_DIR/android.jar --emit-ids ids.txt $res || exit
    let mut status = Command::new(Path::new(&config.tools_dir).join("aapt2"))
        .arg("link")
        .arg("-o")
        .arg(Path::new(&config.tools_dir).join("unaligned.apk"))
        .arg("--manifest")
        .arg("AndroidManifest.xml")
        .arg("-I")
        .arg(&config.android_jar_path)
        .arg("--emit-ids")
        .arg("ids.txt")
        .status()
        .expect("Failed to run aapt2");

    // Append classes file, libraries onto the apk.
    let file = std::fs::File::create(Path::new(&config.android_build_dir).join("unaligned.apk")).unwrap();
    let mut zip = zip::ZipWriter::new(file);


    for (configure_preset, _build_preset) in presets
    {
    }

    //zip.start_file_aligned(name, options, 4);
}

fn main()
{
    std::env::set_current_dir("../").unwrap();

    let config : Configuration = Configuration { 
        cmakeliststxt_dir: "../".to_string(),
        tools_dir: "C:/Users/jofisher/AppData/Local/Android/Sdk/build-tools/34.0.0".to_string(),
        java_code_dir: "src".to_string(),
        android_build_dir: "build".to_string(),
        android_jar_path: "C:/Users/jofisher/AppData/Local/Android/Sdk/platforms/android-33/android.jar".to_string()
    };

    let mut presets : Vec<(String, String)> = Vec::new();
    presets.push(("android-x86_64".to_string(), "android-x86_64-release".to_string()));
    presets.push(("android-x86".to_string(), "android-x86-release".to_string()));
    presets.push(("android-arm64".to_string(), "android-arm64-release".to_string()));
    presets.push(("android-armNeon".to_string(), "android-armNeon-release".to_string()));

    //run_cmake_configure_preset(&config, &presets);
    run_javac(&config);
    run_d8(&config);
}

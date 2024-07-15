# Need to set the directory our CLR calls will run from. For some reason it's different
# than where the script runs from.
[Environment]::CurrentDirectory = $PWD

$fileSizes = @()
$fileData = [System.Text.StringBuilder]::new()

[void]$fileData.AppendLine('// Generated file - do not edit')
[void]$fileData.AppendLine('')
[void]$fileData.AppendLine('#include <string.h>')
[void]$fileData.AppendLine('#include "resources.h"')
[void]$fileData.AppendLine('')

for ($i = 1; $i -lt $args.count; $i++)
{
    Write-Host $args[$i]
    [void]$fileData.AppendLine("static const unsigned char resource_${i}_buffer[] = {");
    [void]$fileData.Append("`t");
    $bytes = [System.IO.File]::ReadAllBytes($args[$i]);
    $fileSizes += $bytes.count

    $byteCounter = 0;
    foreach ($byte in $bytes)
    {
        $formattedByte = ([string]$byte).PadLeft(3, ' ');
        [void]$fileData.Append("${formattedByte}, ");

        $byteCounter++
        if ($byteCounter -eq 16)
        {
            [void]$fileData.AppendLine('');
            [void]$fileData.Append("`t");
            $byteCounter = 0;
        }
    }

    [void]$fileData.AppendLine("")
    [void]$fileData.AppendLine("};`n")
}

[void]$fileData.AppendLine("static const InternalResource resource_list[] = {")

for ($i = 1; $i -lt $args.count; $i++)
{
    $fileName = $args[$i]
    $fileSize = $fileSizes[$i - 1]

    [void]$fileData.AppendLine("`t{ `"${fileName}`", resource_${i}_buffer, ${fileSize} },")
}

[void]$fileData.AppendLine("`t{ 0, 0, 0 }")
[void]$fileData.AppendLine("};")
[void]$fileData.AppendLine("")
[void]$fileData.AppendLine("const InternalResource* getResource(const char* filename)")
[void]$fileData.AppendLine("{")
[void]$fileData.AppendLine("`tfor (const InternalResource* rv = resource_list; rv->buffer; ++rv) {")
[void]$fileData.AppendLine("`t`tif (strcmp(rv->filename, filename) == 0) return rv;")
[void]$fileData.AppendLine("`t}")
[void]$fileData.AppendLine("`treturn NULL;")
[void]$fileData.AppendLine("}")

$fileData.ToString() | Out-File -FilePath $args[0] -Encoding UTF8

 

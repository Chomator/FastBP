# FastBP

> Original idea by [SlothBP](https://github.com/x64dbg/SlothBP): https://github.com/x64dbg/SlothBP

FastBP is a X64DBG plugin that allows you to set breakpoints quickly and efficiently. It supports custom JSON configurations to define breakpoints for various API categories, such as memory operations, file I/O, and more.

![](https://i.imgur.com/LL6Vryx.png)

---

## Key Features

- **Quick Breakpoint Management**: Easily set or clear breakpoints using pre-defined configurations.
- **Customizable JSON**: Define your own API categories and breakpoint mappings using a simple JSON format.
- **Integration with x32/x64dbg**: Drop the plugin into your debugger's plugin directory and start using it immediately.

---

## Example JSON Configuration

The plugin uses a JSON file to define API categories and their corresponding breakpoints. Below is an example of the JSON structure:

```json
{
    "example": {
        "Memory": {
            "Apis": [
                {
                    "Name": "VirtualAlloc",
                    "DllFunction": "kernel32.dll:VirtualAlloc",
                    "Description": "Allocates a region of memory and sets its protection attributes",
                    "ReturnValue": {
                        "Type": "LPVOID",
                        "Description": "address of allocated memory"
                    },
                    "Parameters": "LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect",
                    "Extensions": {}
                },
                {
                    "Name": "VirtualFree",
                    "DllFunction": "kernel32.dll:VirtualFree",
                    "Description": "Frees a region of memory previously allocated byVirtualAllocorVirtualAllocEx",
                    "ReturnValue": {
                        "Type": "BOOL",
                        "Description": "TRUE if successful, FALSE if failed"
                    },
                    "Parameters": "LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType",
                    "Extensions": {}
                }
            ],
            "ALL": [
                "VirtualAlloc",
                "VirtualFree"
            ]
        }
    }
}
```

### JSON Structure Explained

You can also generate JSON from an XLSX file using **xlsx2json.py**.

By default, **xlsx2json.py** reads **apis.xlsx**, but you can specify a different file by running:  
```bash
xlsx2json.py YourFile.xlsx
```

The example **apis.xlsx** file looked like this, and the **Extensions** tag is optional.

| DLL          | API Name         | Description                                                  | Return Value                               | Parameters                                                   | Extensions |
| ------------ | ---------------- | ------------------------------------------------------------ | ------------------------------------------ | ------------------------------------------------------------ | ---------- |
| kernel32.dll | VirtualAlloc     | Allocates a region of memory and sets its protection attributes | LPVOID (address of allocated memory)       | LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect |            |
| kernel32.dll | VirtualFree      | Frees a region of memory previously allocated by `VirtualAlloc` or `VirtualAllocEx` | BOOL (TRUE if successful, FALSE if failed) | LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType            |            |
| kernel32.dll | VirtualProtect   | Changes the protection attributes of a specified region of memory | BOOL (TRUE if successful, FALSE if failed) | LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpOldProtect |            |
| kernel32.dll | VirtualQuery     | Retrieves information about a range of pages in the virtual address space | SIZE_T (size of the queried memory region) | LPVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength |            |
| kernel32.dll | VirtualAllocEx   | Allocates a region of memory within the address space of a specified process and sets its protection attributes | LPVOID (address of allocated memory)       | HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect |            |
| kernel32.dll | VirtualFreeEx    | Frees a region of memory previously allocated by `VirtualAllocEx` | BOOL (TRUE if successful, FALSE if failed) | HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType |            |
| kernel32.dll | VirtualProtectEx | Changes the protection attributes of a region of memory within the address space of a specified process | BOOL (TRUE if successful, FALSE if failed) | HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpOldProtect |            |
| kernel32.dll | VirtualQueryEx   | Retrieves information about a range of pages within the address space of a specified process | SIZE_T (size of the queried memory region) | HANDLE hProcess, LPVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength |            |



---

## How to Use

### Installation

1. **Download the Plugin**:
   - Get the latest release from the [Releases](https://github.com/Chomator/FastBP/releases) page.
2. **Install the Plugin**:
   - Place the downloaded plugin file in the `plugins` directory of your x32/x64dbg installation:
     - For 32-bit: `x32dbg/plugins/`
     - For 64-bit: `x64dbg/plugins/`
3. **Restart x32/x64dbg**:
   - Launch the debugger, and the plugin will be automatically loaded.

### Setting Breakpoints

- Open the plugin's menu in x32/x64dbg.
- Use the `"Set All"` option to set breakpoints for all APIs in the subcategory, or choose individual APIs.

### Customizing Breakpoints

- update xlsx file, and generate JSON with xlsx2json.py.(suggestion)
- Modify the JSON configuration file to add or remove APIs as needed.
- Copy JSON file to the plugin, and reload config, or restart the debugger, to apply changes.

---

## Notes

- **JSON Validation**:
  - Make sure the JSON file is properly formatted when you first run the plugin. You can use tools like [JSONLint](https://jsonlint.com/) to validate your configuration.
  - If you generate JSON file with xlsx2json.py, don't worry, it works fine.

---

## Build

1. **Install nlohmann-json:**  
   Run the following commands to install `nlohmann-json` using vcpkg:  
   ```sh
   vcpkg install nlohmann-json:x86-windows  
   vcpkg install nlohmann-json:x64-windows  
   ```
   For more information about vcpkg, visit: [https://github.com/microsoft/vcpkg](https://github.com/microsoft/vcpkg)  

2. **Open and build the project:**  
   Open `FastBP.sln` in **Visual Studio 2019** and build the project.  
   You can also build it with **Visual Studio 2022**, but it has not been tested.  

---

## License

This project is licensed under the [MIT License](LICENSE). Feel free to use, modify, and distribute it as needed.

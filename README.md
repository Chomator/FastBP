# FastBP

> Original idea by [SlothBP](https://github.com/x64dbg/SlothBP): https://github.com/x64dbg/SlothBP

FastBP is a plugin that allows you to set breakpoints quickly and efficiently. It supports custom JSON configurations to define breakpoints for various API categories, such as memory operations, file I/O, and more.

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
  "Memory": {
    "Basic": {
      "ALL": ["VirtualAlloc", "VirtualFree"],
      "VirtualAlloc": ["kernel32:VirtualAlloc", "Allocate memory"],
      "VirtualFree": ["kernel32:VirtualFree", "Free allocated memory"]
    },
    "Heap": {
      "ALL": ["HeapAlloc", "HeapFree"],
      "HeapAlloc": ["kernel32:HeapAlloc", "Allocate from heap"],
      "HeapFree": ["kernel32:HeapFree", "Free from heap"]
    }
  },
  "File": {
    "Read": {
      "ALL": ["ReadFile", "ReadFileEx"],
      "ReadFile": ["kernel32:ReadFile", "Read data from file"],
      "ReadFileEx": ["kernel32:ReadFileEx", "Asynchronous read from file"]
    },
    "Write": {
      "ALL": ["WriteFile", "WriteFileEx"],
      "WriteFile": ["kernel32:WriteFile", "Write data to file"],
      "WriteFileEx": ["kernel32:WriteFileEx", "Asynchronous write to file"]
    }
  }
}
```

### JSON Structure Explained

- **Top-Level Keys** (e.g., `"Memory"`, `"File"`): Represent high-level categories of APIs.
- **Subcategories** (e.g., `"Basic"`, `"Heap"`): Group related APIs within a category.
- **API Entries**:
  - `"ALL"`: A list of all APIs in the subcategory. Used for batch operations like "Set All" or "Clear All".
  - Individual API keys (e.g., `"VirtualAlloc"`, `"ReadFile"`): Each key maps to an array containing:
    1. The API signature (e.g., `"kernel32:VirtualAlloc"`).
    2. A description of the API's purpose (e.g., `"Allocate memory"`).

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
- Select the desired category (e.g., `"Memory"`) and subcategory (e.g., `"Basic"`).
- Use the `"Set All"` option to set breakpoints for all APIs in the subcategory, or choose individual APIs.

### Customizing Breakpoints

- Modify the JSON configuration file to add or remove APIs as needed.
- Save the updated JSON file and reload config, or restart the debugger, to apply changes.

---

## Notes

- **`Set All` and `Clear All` Actions**:
  - These actions are tied to the `"ALL"` tag in the JSON file. Ensure that the `"ALL"` key is correctly defined for each subcategory.
- **JSON Validation**:
  - Make sure the JSON file is properly formatted. You can use tools like [JSONLint](https://jsonlint.com/) to validate your configuration.

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

import pandas as pd
import json
import os
import sys

def excel_to_json(excel_file="apis.xlsx"):
    """
    Converts an Excel file to a JSON file.
    The JSON file will be named after the Excel file (without extension) + '.json'.
    """
    try:
        # Check if the input file exists
        if not os.path.isfile(excel_file):
            raise FileNotFoundError(f"Error: File '{excel_file}' not found.")
        
        # Read all sheets from the Excel file
        try:
            sheets = pd.read_excel(excel_file, sheet_name=None)
        except Exception as e:
            raise ValueError(f"Error reading Excel file '{excel_file}': {e}")
        
        # Extract the base name of the Excel file (without extension) to use as the root node
        root_node_name = os.path.splitext(os.path.basename(excel_file))[0]
        
        # Build the JSON data structure
        json_data = {}
        all_apis = []  # List to store all API names across all sheets
        
        for sheet_name, df in sheets.items():
            apis = []
            sheet_apis = []  # List to store API names for this specific sheet
            
            for _, row in df.iterrows():
                try:
                    # Retain the original Parameters content without splitting
                    parameters = row['Parameters'].strip() if isinstance(row['Parameters'], str) else ""
                    
                    # Parse Extensions column (JSON format)
                    extensions = {}
                    if 'Extensions' in row and isinstance(row['Extensions'], str) and row['Extensions'].strip():
                        try:
                            extensions = json.loads(row['Extensions'].strip())
                        except json.JSONDecodeError:
                            print(f"Warning: Invalid JSON in Extensions for API '{row['API Name']}' in sheet '{sheet_name}'. Skipping...")
                    
                    # Add API information
                    api = {
                        "Name": row['API Name'],
                        "DllFunction": f"{row['DLL']}:{row['API Name']}",
                        "Description": row['Description'],
                        "ReturnValue": {
                            "Type": row['Return Value'].split('(')[0].strip(),
                            "Description": row['Return Value'].split('(')[-1].rstrip(')').strip()
                        },
                        "Parameters": parameters,  # Directly copy the Parameters content
                        "Extensions": extensions   # Add parsed Extensions
                    }
                    apis.append(api)
                    sheet_apis.append(row['API Name'])
                except KeyError as e:
                    raise KeyError(f"Missing required column '{e}' in sheet '{sheet_name}'.")
                except Exception as e:
                    print(f"Error processing row in sheet '{sheet_name}': {e}. Skipping...")
            
            # Add the API list to the category (sheet name)
            json_data[sheet_name] = {
                "Apis": apis,
                "ALL": sorted(sheet_apis)  # Only includes API names from the current sheet
            }
            
            # Extend the global all_apis list
            all_apis.extend(sheet_apis)
        
        # Wrap the entire structure under the root node (filename without extension)
        final_json = {root_node_name: json_data}
        
        # Generate JSON file name
        json_file = f"{root_node_name}.json"
        
        # Write the JSON data to the output file
        try:
            with open(json_file, 'w', encoding='utf-8') as f:
                json.dump(final_json, f, ensure_ascii=False, indent=4)
            print(f"JSON file successfully created: {json_file}")
        except IOError as e:
            raise IOError(f"Error writing to JSON file '{json_file}': {e}")
    
    except Exception as e:
        print(f"An error occurred: {e}")
        sys.exit(1)

# Example usage
if __name__ == "__main__":
    # If no argument is provided, default to "apis.xlsx"
    if len(sys.argv) > 1:
        excel_file = sys.argv[1]
    else:
        excel_file = "apis.xlsx"
    
    excel_to_json(excel_file)
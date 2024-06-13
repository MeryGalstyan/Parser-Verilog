import json

# Open the JSON file
with open('ontology.json', 'r') as file:
    # Read the contents of the file
    json_str = file.read()

    # Parse the JSON string
    data = json.loads(json_str)

    # Convert it to pretty JSON
    pretty_json = json.dumps(data, indent=4)

    # Print the pretty JSON or write it to another file
    print(pretty_json)


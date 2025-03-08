import os
import toml

secrets_path = "/workspaces/shazam-cpp/.streamlit/secrets.toml"

if os.path.exists(secrets_path):
    secrets = toml.load(secrets_path)
    os.environ["MONGO_URI"] = secrets["secrets"]["MONGO_URI"]
else:
    raise FileNotFoundError("secrets.toml file not found!")

# Debugging
print("Loaded Mongo URI:", os.environ["MONGO_URI"])

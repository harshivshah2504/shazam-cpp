import streamlit as st
import tempfile
import os
import subprocess


import streamlit as st
import tempfile
import os
import subprocess

# Function to check if an executable exists
def check_executable(path):
    return os.path.exists(path) and os.access(path, os.X_OK)

# Function to run `./shazam` with an audio file
def find_song(audio_path):
    shazam_exe = "build/shazam"

    if not check_executable(shazam_exe):
        return 1, f"Error: {shazam_exe} not found or not executable!"

    command = [shazam_exe, audio_path]
    st.write(f"Running command: `{command}`")  # Debugging info

    result = subprocess.run(command, capture_output=True, text=True)

    if result.returncode != 0:
        st.error(f"Error executing `{shazam_exe}`: {result.stderr}")

    return result.returncode, result.stdout

# Function to run `./add` with song details
def add_song(file_path, song_name, artist_name):
    add_exe = "build/add"

    if not check_executable(add_exe):
        return 1, f"Error: {add_exe} not found or not executable!"

    command = [add_exe, file_path, song_name, artist_name]
    st.write(f"Running command: `{command}`")  # Debugging info

    result = subprocess.run(command, capture_output=True, text=True)

    if result.returncode != 0:
        st.error(f"Error executing `{add_exe}`: {result.stderr}")

    return result.returncode, result.stdout


# Streamlit UI
st.title("🎵 SeekTune")
st.subheader("Find songs by uploading an audio file or adding new songs to the database.")

# Create Tabs: Upload & Match | Add New Song
tab1, tab2 = st.tabs(["Upload & Match", "Add New Song"])

# 🎤 Tab 1: Upload Audio & Match
with tab1:
    st.subheader("Upload & Match a Song")
    uploaded_file = st.file_uploader("Upload an MP3 file", type=["mp3"])
    
    if uploaded_file is not None:
        with tempfile.NamedTemporaryFile(delete=False, suffix=".mp3") as temp_mp3:
            temp_mp3.write(uploaded_file.read())
            temp_mp3_path = temp_mp3.name

        st.success("File uploaded successfully!")
        st.audio(temp_mp3_path, format='audio/mp3')

        if st.button("🔍 Match Song"):
            with st.spinner("Matching song..."):
                return_code, output = find_song(temp_mp3_path)
                if return_code == 0:
                    st.success("Match Found!")
                else:
                    st.error("No match found or an error occurred.")
                st.write("**Match Result:**", output)
                os.remove(temp_mp3_path)  # Cleanup temp file

# 🎵 Tab 2: Add a New Song to Database
with tab2:
    st.subheader("Add a New Song")
    song_file = st.file_uploader("Upload an MP3 file to add", type=["mp3"], key="add_song")
    song_name = st.text_input("Enter Song Name")
    artist_name = st.text_input("Enter Artist Name")

    if song_file and song_name and artist_name:
        with tempfile.NamedTemporaryFile(delete=False, suffix=".mp3") as temp_song:
            temp_song.write(song_file.read())
            temp_song_path = temp_song.name

        if st.button("Submit"):
            with st.spinner("Adding song to database..."):
                return_code, output = add_song(temp_song_path, song_name, artist_name)
                st.write(return_code)
                if return_code == 0:
                    st.success("Song added successfully!")
                else:
                    st.error("Failed to add song.")
                st.text(output)
                os.remove(temp_song_path)  # Cleanup temp file

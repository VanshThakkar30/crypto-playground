# In your routes.py file

from app import app
# Make sure to import current_app from flask
from flask import render_template, current_app

@app.route('/')
def index():
    """Renders the main application page."""
    return render_template('index.html')

# --- ADD THIS NEW FUNCTION ---
@app.route('/debug-static')
def debug_static():
    path = current_app.static_folder
    print(f"--- FLASK DEBUG ---")
    print(f"The static folder path is: {path}")
    print(f"--------------------")
    return f"Flask is looking for the static folder at this location on your computer: <br><strong>{path}</strong>"
# -------------------------
from flask import Flask, request, jsonify
from datetime import datetime, timedelta
import mysql.connector

# ============================
# DB CONFIG
# ============================
db_config = {
    'user': 'your_username',
    'password': 'your_password',
    'host': 'your_database_host',
    'database': 'your_database_name'
}

def get_db():
    return mysql.connector.connect(**db_config)

# ============================
# Helper Functions
# ============================
def clean(value):
    if value in ["", "nan", "null", None]:
        return None
    return value

# ============================
# Flask App
# ============================
app = Flask(__name__)

# ============================
# API — GET TIME (GMT+7)
# ============================
@app.route("/api/time", methods=["GET"])
def api_time():
    now_utc = datetime.utcnow()
    now_gmt7 = now_utc + timedelta(hours=7)
    return jsonify({
        "datetime": now_gmt7.strftime("%Y-%m-%d %H:%M:%S"),
        "timezone": "GMT+7"
    })

# ============================
# API — POST DATA IoT
# ============================
@app.route("/api/post/data", methods=["POST"])
def api_post():
    data = request.json

    device = clean(data.get("device"))
    sensor1 = clean(data.get("sensor1"))
    sensor2 = clean(data.get("sensor2"))
    created_at = clean(data.get("created_at"))

    conn = get_db()
    cursor = conn.cursor()

    table = "your_table_name"

    query = f"""
        INSERT INTO {table} (device, sensor1, sensor2, created_at)
        VALUES (%s, %s, %s, %s)
    """

    cursor.execute(query, (device, sensor1, sensor2, created_at))
    conn.commit()

    cursor.close()
    conn.close()

    return jsonify({"message": "Data inserted successfully"})

# ============================
# GET ALL DATA BY DEVICE
# ============================
@app.route('/api/get/data/<device>', methods=['GET'])
def get_data(device):

    conn = get_db()
    cursor = conn.cursor()

    table_name = "your_table_name"

    query = f"""
        SELECT id, device, sensor1, sensor2 created_at
        FROM {table_name}
        WHERE device = %s
        ORDER BY created_at
    """

    cursor.execute(query, (device,))
    rows = cursor.fetchall()
    columns = [col[0] for col in cursor.description]

    cursor.close()
    conn.close()

    # Format JSON response
    result = []
    for row in rows:
        item = {}
        for col, val in zip(columns, row):
            if isinstance(val, datetime):
                item[col] = val.strftime('%Y-%m-%d %H:%M:%S')
            else:
                item[col] = val
        result.append(item)

    return jsonify(result)

# ============================
# RUN FLASK
# ============================
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)

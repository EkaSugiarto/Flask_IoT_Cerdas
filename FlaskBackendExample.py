from flask import Flask, request, jsonify
from datetime import datetime, timedelta
import mysql.connector

# ============================
# DB CONFIG
# ============================
db_config = {
    'user': 'kelasiotcerdas',
    'password': 'labae123456',
    'host': 'kelasiotcerdas.mysql.pythonanywhere-services.com',
    'database': 'kelasiotcerdas$database_perangkat_iot'
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
    co2 = clean(data.get("co2"))
    suhu = clean(data.get("suhu"))
    kelembapan = clean(data.get("kelembapan"))
    created_at = clean(data.get("created_at"))

    conn = get_db()
    cursor = conn.cursor()

    table = "data_aqms"

    query = f"""
        INSERT INTO {table} (device, co2, suhu, kelembapan, created_at)
        VALUES (%s, %s, %s, %s, %s)
    """

    cursor.execute(query, (device, co2, suhu, kelembapan, created_at))
    conn.commit()

    cursor.close()
    conn.close()

    return jsonify({"message": "Data inserted successfully"})

# ============================
# GET DATA BY DEVICE
# ============================
@app.route('/api/get/data/<device>', methods=['GET'])
def get_data(device):

    conn = get_db()
    cursor = conn.cursor()

    table_name = "data_aqms"

    now_utc = datetime.utcnow()
    now_gmt7 = (now_utc + timedelta(hours=7)).replace(second=0, microsecond=0)

    h2_before = (now_gmt7 - timedelta(hours=2)).replace(second=0, microsecond=0)


    if 'h2' in request.args:
        query = f"""
            SELECT id, device, co2, suhu, kelembapan, created_at
            FROM {table_name}
            WHERE device = %s
            AND created_at BETWEEN %s AND %s
            ORDER BY created_at
        """
        params = (
            device,
            h2_before.strftime("%Y-%m-%d %H:%M:%S"),
            now_gmt7.strftime("%Y-%m-%d %H:%M:%S")
        )

    elif 'last10' in request.args:
        query = f"""
            SELECT id, device, co2, suhu, kelembapan, created_at
            FROM {table_name}
            WHERE device = %s
            ORDER BY created_at DESC
            LIMIT 10
        """
        params = (device,)

    elif 'last_data' in request.args:
        query = f"""
            SELECT id, device, co2, suhu, kelembapan, created_at
            FROM {table_name}
            WHERE device = %s
            ORDER BY created_at DESC
            LIMIT 1
        """
        params = (device,)

    else:
        query = f"""
            SELECT id, device, co2, suhu, kelembapan, created_at
            FROM {table_name}
            WHERE device = %s
            ORDER BY created_at
        """
        params = (device,)


    cursor.execute(query, params)
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
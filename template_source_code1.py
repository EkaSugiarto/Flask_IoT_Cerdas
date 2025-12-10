from flask import Blueprint, request, jsonify
from datetime import datetime, timedelta
from db_config import get_db_connection

api = Blueprint('api', __name__)

# Helper untuk bersihin input
def clean_data(value):
    if value in ["", "nan", "null", None]:
        return None
    return value

# ============================
# Endpoint GET Time GMT+7
# ============================
@api.route('/api/time', methods=['GET'])
def get_time():
    now_utc = datetime.utcnow()
    now_gmt7 = now_utc + timedelta(hours=7)
    return jsonify({
        'datetime': now_gmt7.strftime('%Y-%m-%d %H:%M:%S'),
        'timezone': 'GMT+7'
    })


# ============================
# Endpoint POST data IoT
# ============================
@api.route('/api/post/data', methods=['POST'])
def post_data():

    data = request.json

    device = clean_data(data.get("device"))
    sensor1 = clean_data(data.get("sensor1"))
    created_at = clean_data(data.get("created_at"))

    conn = get_db_connection()
    cursor = conn.cursor()

    table_name = "your_database_table_name"

    query = f"""
        INSERT INTO {table_name} (
            device, sensor1, created_at
        ) VALUES (%s, %s, %s)
    """

    cursor.execute(query, (device, sensor1, created_at))
    conn.commit()

    cursor.close()
    conn.close()

    return jsonify({"message": "Data inserted successfully"})


# ============================
# Endpoint GET data IoT (ALL DATA)
# /api/get/data/<device>
# ============================
@api.route('/api/get/data/<device>', methods=['GET'])
def get_data(device):

    conn = get_db_connection()
    cursor = conn.cursor()

    table_name = "your_database_table_name"

    query = f"""
        SELECT id, device, sensor1, created_at
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


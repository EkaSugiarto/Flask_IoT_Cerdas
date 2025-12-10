import mysql.connector

db_config = {
    'user': 'YOUR_DB_USER',
    'password': 'YOUR_DB_PASSWORD',
    'host': 'YOUR_DB_HOST',
    'database': 'YOUR_DB_NAME'
}

def get_db_connection():
    return mysql.connector.connect(**db_config)

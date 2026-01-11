import streamlit as st
import requests
import pandas as pd
import plotly.express as px
from streamlit_autorefresh import st_autorefresh

# ======================
# PAGE CONFIG
# ======================
st.set_page_config(
    page_title="IoT Dashboard",
    layout="wide"
)

# Auto refresh tiap 2 menit
st_autorefresh(
    interval=120_000,
    key="iot_autorefresh"
)

st.title("AQMS IoT Monitoring Dashboard")

# ======================
# DEVICE CONFIG
# ======================
DEVICE_MAP = {
    "OAQMS_GKU"
}

BASE_URL = "https://kelasiotcerdas.pythonanywhere.com/api/get/data"

# ======================
# TOP CONTROL
# ======================
col_ctrl, _ = st.columns([2, 8])

with col_ctrl:
    station = st.selectbox(
        "Pilih Station",
        list(DEVICE_MAP)
    )

# ======================
# API FUNCTIONS
# ======================
@st.cache_data(ttl=60)
def load_last_data(station):
    url = f"{BASE_URL}/{station}?last_data"
    res = requests.get(url, timeout=10)
    res.raise_for_status()
    return pd.DataFrame(res.json())

@st.cache_data(ttl=120)
def load_h2_data(station):
    url = f"{BASE_URL}/{station}?h2"
    res = requests.get(url, timeout=10)
    res.raise_for_status()
    return pd.DataFrame(res.json())

# ======================
# LOAD DATA
# ======================
df_last = load_last_data(station)
df_h2 = load_h2_data(station)

if df_last.empty:
    st.warning("Data belum tersedia!")
    st.stop()

# ======================
# LAST DATA PROCESS
# ======================
df_last["created_at"] = pd.to_datetime(df_last["created_at"])
df_last = df_last.sort_values("created_at")

latest = df_last.iloc[-1]

# ======================
# METRICS
# ======================
col1, col2, col3 = st.columns(3)

col1.metric("CO₂ (ppm)", f"{latest['co2']}")
col2.metric("Suhu (°C)", f"{latest['suhu']}")
col3.metric("Kelembapan (%)", f"{latest['kelembapan']}")

# ======================
# LAST UPDATED
# ======================
with st.container(border=True):
    st.caption(f"Last updated: {latest['created_at']}")

# ======================
# PARAM SELECT
# ======================
param = st.radio(
    "Pilih Parameter Grafik (data 2 Jam Terakhir)",
    ["co2", "suhu", "kelembapan"],
    horizontal=True
)

if df_h2.empty:
    st.warning("Data 2 jam belum tersedia!")
    st.stop()

# ======================
# H2 DATA PROCESS
# ======================
df_h2["created_at"] = pd.to_datetime(df_h2["created_at"])
df_h2 = df_h2.sort_values("created_at")


# ======================
# AUTO FILL 2 MENIT
# ======================
df_plot = df_h2[["created_at", param]].copy()
df_plot[param] = pd.to_numeric(df_plot[param], errors="coerce")

df_plot = df_plot.set_index("created_at")

time_index = pd.date_range(
    start=df_plot.index.min(),
    end=df_plot.index.max(),
    freq="2min"
)

df_plot = df_plot.reindex(time_index)

df_plot = df_plot.reset_index()
df_plot.rename(columns={"index": "created_at"}, inplace=True)


# ======================
# PLOTLY GRAPH
# ======================
fig = px.line(
    df_plot,
    x="created_at",
    y=param,
    markers=True,
    title=f"{param.upper()} - {station} (2 Jam Terakhir)"
)

fig.update_layout(
    xaxis=dict(
        title="Waktu",
        showgrid=True,
        dtick=120000,
        tickformat="%H:%M",
        tickangle=-45
    ),
    yaxis=dict(
        title=param.upper(),
        showgrid=True
    ),
    height=450
)

st.plotly_chart(fig, use_container_width=True)
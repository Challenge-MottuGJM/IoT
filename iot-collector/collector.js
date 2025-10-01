import mqtt from 'mqtt';
import Database from 'better-sqlite3';
import fs from 'node:fs';
import path from 'node:path';

const db = new Database('iot_events.db');
db.exec(`
  CREATE TABLE IF NOT EXISTS events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    deviceId TEXT,
    sector TEXT,
    present INTEGER,
    ts INTEGER,
    received_at INTEGER
  );
  CREATE INDEX IF NOT EXISTS idx_events_time ON events(ts);
`);

const csvPath = path.join(process.cwd(), 'events.csv');
if (!fs.existsSync(csvPath)) fs.writeFileSync(csvPath, 'deviceId,sector,present,ts,received_at\n', 'utf8');

const insert = db.prepare('INSERT INTO events (deviceId, sector, present, ts, received_at) VALUES (@deviceId,@sector,@present,@ts,@received_at)');

function saveEvent(e) {
  insert.run({ ...e, present: e.present ? 1 : 0, received_at: Math.floor(Date.now()/1000) });
  const line = `${e.deviceId},${e.sector},${e.present ? 1 : 0},${e.ts},${Math.floor(Date.now()/1000)}\n`;
  fs.appendFileSync(csvPath, line, 'utf8');
}

const client = mqtt.connect('mqtt://test.mosquitto.org:1883');
client.on('connect', () => {
  console.log('MQTT conectado');
  client.subscribe('motttu/telemetry/+');
});
client.on('message', (topic, payload) => {
  try {
    const msg = JSON.parse(payload.toString());
    if (msg?.deviceId && msg?.sector && typeof msg?.present === 'boolean') {
      saveEvent(msg);
      console.log('EVENTO', msg);
    }
  } catch (err) {
    console.error('JSON inválido', err);
  }
});

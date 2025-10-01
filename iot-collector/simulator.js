import mqtt from 'mqtt';

const client = mqtt.connect('mqtt://test.mosquitto.org:1883');

const devices = [
  { deviceId: 'espA', sector: 'analise' },
  { deviceId: 'espB', sector: 'manutencao' },
  { deviceId: 'espC', sector: 'liberadas' }
];

client.on('connect', () => {
  console.log('Simulador conectado');
  setInterval(() => {
    const d = devices[Math.floor(Math.random() * devices.length)];
    const present = Math.random() > 0.5;
    const msg = {
      deviceId: d.deviceId,
      sector: d.sector,
      present,
      ts: Math.floor(Date.now() / 1000)
    };
    client.publish(`motttu/telemetry/${d.deviceId}`, JSON.stringify(msg));
    console.log('PUB', msg);
  }, 7000);
});

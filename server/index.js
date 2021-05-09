require('dotenv').config();
const express = require('express');
const cors = require('cors');
const app = express();

const port = process.env.PORT;
//State
let candleIsOpen = true;
let toggleTimestamp = 0;

const corsOptions = {
  origin: 'https://ivhuc.grafana.net/',
};

//Routes
app.get('/', (_, res) => {
  res.send('Welcome to 🕯 monitoring!');
});

app.get('/toggle', cors(), (req, res) => {
  const canToggle = toggleTimestamp < Date.now();
  const isAuthorized = req.headers['secret-token'] === process.env.SECRET_TOKEN;
  if (isAuthorized && canToggle) {
    toggleTimestamp = Date.now() + 15000;
    candleIsOpen = !candleIsOpen;
    res.send(`Toggled state to ${candleIsOpen ? 'opened' : 'closed'}`);
  } else {
    res.send(`Can't toggle state`);
  }
});

app.get('/toggleGrafana', cors(corsOptions), (req, res) => {
  const canToggle = toggleTimestamp < Date.now();
  if (canToggle) {
    toggleTimestamp = Date.now() + 15000;
    candleIsOpen = !candleIsOpen;
    res.send(`Toggled state to ${candleIsOpen ? 'opened' : 'closed'}`);
  } else {
    res.send(`Can't toggle state`);
  }
});

app.get('/status', cors(), (_, res) => {
  res.send({ candleIsOpen: `${candleIsOpen ? 1 : 0}` });
});

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`);
});

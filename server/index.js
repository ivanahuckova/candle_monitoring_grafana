require('dotenv').config();
const express = require('express');
const port = process.env.PORT || 9000;
const app = express();

//State
let candleIsOpen = true;
let toggleTimestamp = 0;

//Routes
app.get('/', (_, res) => {
  res.send('Welcome to 🕯 monitoring!');
});

app.get('/toggle', (req, res) => {
  const canToggle = toggleTimestamp < Date.now();
  const isAuthorized = req.headers.authorization === process.env.AUTH_HEADER;
  if (isAuthorized && canToggle) {
    toggleTimestamp = Date.now() + 15000;
    candleIsOpen = !candleIsOpen;
    res.send(`Toggled state to ${candleIsOpen ? 'opened' : 'closed'}`);
  } else {
    res.send(`Can't toggle state`);
  }
});

app.get('/status', (_, res) => {
  res.send({ candleIsOpen: `${candleIsOpen ? 1 : 0}` });
});

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`);
});

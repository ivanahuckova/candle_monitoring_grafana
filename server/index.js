require('dotenv').config();
const express = require('express');
const cors = require('cors');
const port = process.env.PORT;
const app = express();

//State
let candleIsOpen = true;
let toggleTimestamp = 0;

//Corse & pre-flight
app.use(cors());
app.options('*', cors());
//Routes

app.get('/', (_, res) => {
  res.send('Welcome to 🕯 monitoring!');
});

app.get('/toggle', (req, res) => {
  const canToggle = toggleTimestamp < Date.now();
  if (canToggle) {
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

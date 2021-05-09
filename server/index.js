require('dotenv').config();
const express = require('express');
const cors = require('cors');
const app = express();

const port = process.env.PORT;
//State
let candleIsOpen = true;
let toggleTimestamp = 0;

//Cors & pre-flight
app.use(cors());

//Routes
app.get('/', (_, res) => {
  res.status(200).send('<h1>Welcome to 🕯 monitoring</h1>');
});

app.get('/toggle', (req, res) => {
  const canToggle = toggleTimestamp < Date.now() && req.query.secret === process.env.SECRET_TOKEN;
  if (canToggle) {
    toggleTimestamp = Date.now() + 15000;
    candleIsOpen = !candleIsOpen;
    res.status(200).json({ message: `Toggled state to ${candleIsOpen ? 'opened' : 'closed'}` });
  } else {
    res.status(403).json({ message: `Can't toggle state` });
  }
});

app.get('/status', (_, res) => {
  res.status(200).json({ candleIsOpen: `${candleIsOpen ? 1 : 0}` });
});

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`);
});

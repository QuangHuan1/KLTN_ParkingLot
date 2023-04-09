const path = require('path');
const express = require('express');
const morgan = require('morgan');
const handlebars = require('express-handlebars');
const route = require('./routes');
const db = require('./config/db');

// Connect to DB
db.connect();

const app = express();
const port = 3000;

// Static files
app.use(express.static(path.join(__dirname, 'public')));

// Middleware for POST method
app.use(
    express.urlencoded({
        extended: true,
    }),
);
app.use(express.json());

// HTTP Logger
// app.use(morgan('combined'))

// Template engine
app.engine('hbs', handlebars.engine({ extname: '.hbs' }));
app.set('view engine', 'hbs');
app.set('views', path.join(__dirname, 'resource', 'views'));

// Routes init
route(app);

app.listen(port, () => {
    console.log(`App listening on port ${port}`);
});

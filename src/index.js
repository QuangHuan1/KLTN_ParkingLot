const path = require('path');
const express = require('express');
const morgan = require('morgan');
const methodOverride = require('method-override');
const handlebars = require('express-handlebars');
const route = require('./routes');
const db = require('./config/db');
const http = require('http');
const cors = require('cors');

// Connect to DB
db.connect();

const app = express();
const port = 3000;

// Static files
app.use(express.static(path.join(__dirname, '/public/')));

// Middleware for POST method
app.use(
    express.urlencoded({
        extended: true,
    }),
);
app.use(express.json());

app.use(cors());

app.use(methodOverride('_method'));

// HTTP Logger
// app.use(morgan('combined'))

// Template engine
app.engine('hbs', handlebars.engine({ extname: '.hbs' }));
app.set('view engine', 'hbs');
app.set('views', path.join(__dirname, 'resource', 'views'));

// Routes init
route(app);

// Socket
const server = http.createServer(app);
const socketIo = require('socket.io')(server, {
    cors: {
        origin: '*',
    },
});
socketIo.on('connection', (socket) => {
    console.log('New client connected: ' + socket.id);
    socket.on('disconnect', () => {
        console.log('Client disconnected');
    });
});

app.io = socketIo;

server.listen(port, () => {
    console.log(`App listening on port ${port}`);
});

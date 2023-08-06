const express = require('express');
const router = express.Router();
const checkinController = require('../app/controllers/check-in-controller');

router.post('/', checkinController.alarm);

module.exports = router;

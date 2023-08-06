const express = require('express');
const router = express.Router();
const confirmCheckinController = require('../app/controllers/confirm-check-in-controller');

//[GET] /confirm-check-in
router.get('/', confirmCheckinController.getConfirmCheckin);

module.exports = router;

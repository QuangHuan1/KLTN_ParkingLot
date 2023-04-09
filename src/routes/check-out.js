const express = require('express');
const router = express.Router();
const checkOutController = require('../app/controllers/check-out-controller');

//[POST] /check-in
router.get('/', checkOutController.getCheckout);

//[POST] /check-out
router.post('/', checkOutController.index);

module.exports = router;

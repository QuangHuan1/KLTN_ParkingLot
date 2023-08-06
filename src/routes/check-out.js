const express = require('express');
const router = express.Router();
const checkOutController = require('../app/controllers/check-out-controller');

//[POST] /check-in
router.get('/', checkOutController.getCheckout);

//[GET] /data
router.get('/data', checkOutController.getDataCheckOuts);

//[GET] /data
router.get('/:id', checkOutController.getDataCheckOut);

//[POST] /check-out
router.post('/', checkOutController.index);

module.exports = router;

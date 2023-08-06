const express = require('express');
const router = express.Router();
const authMiddleware = require('../app/controllers/auth-middleware');
const billController = require('../app/controllers/bill-controller');

router.get('/', billController.show);
router.get('/data', billController.getDataBills);
router.put('/update', authMiddleware.admin, billController.update);
router.get('/fix', billController.fix);

module.exports = router;

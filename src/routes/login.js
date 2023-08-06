const express = require('express');
const router = express.Router();
const loginController = require('../app/controllers/login-controller');

router.get('/', loginController.index);
router.post('/sign-in', loginController.signIn);

module.exports = router;

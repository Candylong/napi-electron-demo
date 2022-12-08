//'use strict'
const addon = require('./CustomAddon')
let myObjWrap2 = null
const testAPI = document.getElementById('TEST')
function tsfn()
{
    console.log('hello world')
}

function callEmit() 
{
    console.log(arguments[0]);
}

testAPI.onclick = () =>
{
    addon.CreateFunction({tsfn: callEmit})
    myObjWrap2 = addon.CreateObject()
}
//'use strict'
let nodepath = ''
let os = require('os')
if(os.platform() === 'darwin')
{
    nodepath = './CustomAddonMac.node';
}
else if(os.platform() === 'win32')
{
    nodepath = './CustomAddonWin.node';
}

const addon = require(nodepath)
let myObjWrap2 = null
const testAPI = document.getElementById('TEST')

function callEmit() 
{
    console.log(arguments[0]);
}

testAPI.onclick = () =>
{
    addon.CreateFunction({tsfn: callEmit})
    myObjWrap2 = addon.CreateObject()
}
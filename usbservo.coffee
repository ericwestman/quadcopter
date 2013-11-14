usb = require 'usb'
prompt = require 'prompt'
spawn = require('child_process').spawn
SerialPort = require("serialport").SerialPort
Deferred = require("promised-io/promise").Deferred;

prompt.start();

COMMANDS = 
	HELLO: 0
	SET_VALS: 1
	GET_VALS: 2
	PRINT_VALS: 3

setup = ->
	defferred = new Deferred()
	pic = usb.findByIds(0x6666, 0x0003)

	unless pic?
		console.log 'no USB device found matching specified vID and pID'
		process.exit(1);

	pic.timeout = 1000
	pic.open()

	setTimeout(->
		defferred.resolve(pic)
	, 1000)

	defferred.promise

executionExample = ->
	pic.controlTransfer(0x40, COMMANDS.HELLO, 0, 0, new Buffer(0), (err,data) ->
		if err
			console.log err
		else
			console.log "u seen helloworld?"
	)

setVals = (pic, val1, val2) ->
	defferred = new Deferred()
	pic.controlTransfer 0x40, COMMANDS.SET_VALS, val1, val2, new Buffer(0), (err,data) ->
		defferred.resolve pic
	defferred.promise


repl = (pic)->
	prompt.get ['val1', 'val2'], (err, result)->
		if err
			console.log err
		console.log "Value 1:#{result.val1}, Value 2:#{result.val2}"
		setVals(pic, result.val1, result.val2).then (pic) ->
			repl pic
		# pic.controlTransfer 0x40, COMMANDS.SET_VALS, result.val1, result.val2, new Buffer(0), (err,data) ->
		# 	repl pic

setup().then (pic) ->
	repl pic



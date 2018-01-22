require 'faye/websocket'
require 'eventmachine'
require 'json'
require_relative './common.rb'

EM.run {
  ws = Faye::WebSocket::Client.new('wss://api.sakura.io/ws/v1/b1e70c4b-f775-42d5-a350-eb48b5bfeb73')

  ws.on :open do |event|
    p [:open]
    ws.send('Hello, world!')
  end

  ws.on :message do |event|
    p [:message, event.data]
    _data = JSON.parse(event.data)
    if _data.has_key?('payload')
      if _data['payload'].has_key?('channels')
        channels = _data['payload']['channels']
        lat, lng = 0, 0
        lat = channels.find{|chan| chan['channel'] == 0 }['value']
        lng = channels.find{|chan| chan['channel'] == 1 }['value']
        $DB[:places].insert(lat: lat, lng: lng, created_at: Time.now)
      end
    end
  end

  ws.on :close do |event|
    p [:close, event.code, event.reason]
    ws = nil
  end
}

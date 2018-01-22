require 'sequel'
require_relative './common.rb'

$DB[:places].insert(lat: 35.694108, lng: 140.695426, created_at: Time.now)
$DB[:places].insert(lat: 38.694108, lng: 139.695426, created_at: Time.now)
$DB[:places].insert(lat: 40.694108, lng: 150.695426, created_at: Time.now)

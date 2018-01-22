require 'sinatra/base'
require 'sinatra/reloader'
require 'sinatra/json'
require 'sequel'
require 'pry'
require 'haml'
require_relative './common.rb'

class App < Sinatra::Base
  configure :development do
    register Sinatra::Reloader
  end

  get '/' do
    haml :index
  end

  get '/places' do
    places = DB[:places].all
    json places
  end

  post '/' do
    lat = params['lat']
    lng = params['lng']

    $DB[:places].insert(lat: lat, lng: lng, created_at: Time.now)
    status 200
  end

  run!
end



-- CCash Exchange Backend
local reserveAccount = "Reserve"
local reservePassword = "testaccount"
local reserveMaximum = 100
local commodity = "minecraft:coal"

-- Backend Routine
os.pullEvent = os.pullEventRaw
local version = 1
local function backend()

    -- Load API
    if not fs.exists("/ccash.lua") then
        h = http.get("https://raw.githubusercontent.com/Reactified/rpm/master/packages/ccash-api/api.lua")
        if h then
            f = fs.open("/ccash.lua","w")
            f.writeLine(h.readAll())
            f.close()
        else
            printError("Failed to download CCash API")
            return
        end
    end
    os.loadAPI("/ccash.lua")
    local api = _G["ccash"]

    -- Networking
    local modem = peripheral.wrap("top") -- WIRED MODEM THAT SHOULD ONLY CONNECT TO FRONTEND
    local port = 8192 -- COMMUNCATION PORT, THIS DOESN'T MATTER MUCH BUT MUST MATCH FRONTEND

    modem.open(port)

    local function recv(timeout)
        local timeoutTimer = false
        if timeout then
            timeoutTimer = os.startTimer(timeout)
        end
        while true do
            local e,s,c,r,m = os.pullEvent()
            if e == "timer" and s == timeoutTimer then
                return false
            elseif e == "modem_message" and c == port then
                print("<< "..tostring(m))
                return m
            end
        end
    end

    local function send(packet)
        modem.transmit(port, port, packet)
        print(">> "..tostring(packet))
    end

    -- Economy
    function exchangeRate()
        return 10
    end

    -- Routine
    local depositAmount = 0
    while true do

        local cmd = recv()

        if cmd == "[PING]" then
            send("[PONG]")
        elseif cmd == "[VERSION-CHECK]" then
            send(version)
        elseif cmd == "[ONLINE-CHECK]" then
            if api.simple.online() then
                send("[ONLINE]")
            else
                send("[OFFLINE]")
            end
        elseif cmd == "[RESERVE-BALANCE-CHECK]" then
            send(api.simple.balance(reserveAccount))
        elseif cmd == "[RESERVE-MAXIMUM-CHECK]" then
            send(reserveMaximum)
        elseif cmd == "[EXCHANGE-RATE-CHECK]" then
            send(exchangeRate())
        elseif cmd == "[DEPOSIT-CHECK]" then
            local slot = 1
            local success = false
            while true do
                turtle.select(slot)
                if turtle.getItemCount() > 0 then
                    slot = slot + 1
                    if slot > 16 then
                        break
                    end
                else
                    turtle.suck()
                    local detail = turtle.getItemDetail()
                    local count = turtle.getItemCount()
                    if not detail then
                        send(success)
                        break
                    end
                    if detail.name == commodity then
                        success = true
                        depositAmount = depositAmount + count
                    end
                end
            end
        elseif cmd == "[DEPOSIT-AMOUNT]" then
            send(depositAmount)
        elseif cmd == "[DEPOSIT-RETURN]" then
            depositAmount = 0
            for i=1,16 do
                turtle.select(i)
                turtle.drop()
            end
        elseif cmd == "[DEPOSIT-CONFIRM]" then
            local username = recv()
            local depositValue = math.floor(depositAmount*exchangeRate())
            local reserveBalance
            repeat
                reserveBalance = api.simple.balance(reserveAccount)
            until reserveBalance
            if reserveBalance >= depositValue then
                write("Attempting deposit... ")
                local success = api.simple.send(reserveAccount, reservePassword, username, depositValue)
                print(success)
                if success then
                    send(true)
                    for i=1,16 do
                        turtle.select(i)
                        turtle.dropDown()
                    end
                    depositAmount = 0
                else
                    send(false)
                end
            else
                send(false)
            end
        end

    end

end

-- Error Handler
while true do
    ok,err = pcall(backend)
    term.setCursorPos(1,1)
    printError(err)
    sleep(5)
end

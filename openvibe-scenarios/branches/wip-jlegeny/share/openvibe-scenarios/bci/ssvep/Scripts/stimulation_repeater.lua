STIMULATION_BASE = 0x00008100
STIMULATION_END = 0x000081ff

function initialize()
	io.write("[Stimulation adaptor] initialized\n")
end

function uninitialize()
	io.write("[Stimulation adaptor] uninitialized\n")
end

function process(box)
	io.write("[Stimulation adaptor] processing\n")

	current_stimulation = 0
	repeater = false
	last_time = 0

	while true do
		time = box:get_current_time()

		for stimulation = 1, box:get_stimulation_count(1) do

			s_code, s_date, s_duration = box:get_stimulation(1, 1)

			if s_code >= STIMULATION_BASE and s_code < STIMULATION_END then
				repeater = true
				current_stimulation = s_code
			elseif s_code == STIMULATION_END then
				repeater = false
			end

			box:remove_stimulation(1, 1)
		end

		if repeater and time >= last_time + 0.1 then
			box:send_stimulation(1, current_stimulation, time)
			--send_stimulation(1, STIMULATION_BASE, time + 0.01)
			last_time = time
		end


		box:sleep()
	end

end

sequence = {}
stimulationDuration = nil
breakDuration = nil
flickeringDelay = nil

Stim_ExperimentStart = 0x00008001
Stim_ExperimentStop = 0x00008002
Stim_VisualStimulationStart = 0x0000800b
Stim_VisualStimulationStop = 0x0000800c

stimulationLabels = {
	0x00008100,
	0x00008101,
	0x00008102,
	0x00008103,
	0x00008104,
	0x00008105,
	0x00008106,
	0x00008107
}

function initialize(box)
	s_sequence = box:get_setting(2)
	s_stimulationDuration = box:get_setting(3)
	s_breakDuration = box:get_setting(4)
	s_flickeringDelay = box:get_setting(5)

	io.write(string.format("Goal Sequence : [%s]\n", s_sequence))
	io.write(string.format("Stimulation Duration : [%s]\n", s_stimulationDuration))
	io.write(string.format("Break Duration : [%s]\n", s_breakDuration))
	io.write(string.format("Flickering Delay : [%s]\n", s_flickeringDelay))

	if (s_stimulationDuration:find("^%d+$") ~= nil) then
		stimulationDuration = tonumber(s_stimulationDuration)
	else
		io.write("[ERROR] The parameter 'stimulation duration' should be a numeric value\n")
	end

	if (s_breakDuration:find("^%d+$") ~= nil) then
		breakDuration = tonumber(s_breakDuration)
	else
		io.write("[ERROR] The parameter 'break duration' should be a numeric value\n")
	end

	if (s_flickeringDelay:find("^%d+$") ~= nil) then
		flickeringDelay = tonumber(s_flickeringDelay)
	else
		io.write("[ERROR] The parameter 'flickering delay' should be a numeric value\n")
	end

	for goal in s_sequence:gmatch("%d+") do
		table.insert(sequence, goal)
	end

end

function uninitialize(box)
end

function process(box)

	while box:get_stimulation_count(1) == 0 do
		box:sleep()
	end

	current_time = box:get_current_time() + 1

	box:send_stimulation(1, Stim_ExperimentStart, current_time, 0)

	current_time = current_time + 2

	for i,goal in ipairs(sequence) do
		io.write(string.format("goal no %d is %d at %d\n", i, goal, current_time))
		-- mark goal
		box:send_stimulation(2, stimulationLabels[goal + 1], current_time, 0)
		-- wait for FlickeringDelay seconds
		current_time = current_time + flickeringDelay
		-- start flickering
		box:send_stimulation(1, Stim_VisualStimulationStart, current_time, 0)
		-- wait for StimulationDuration seconds
		current_time = current_time + stimulationDuration
		-- unmark goal and stop flickering
		box:send_stimulation(1, Stim_VisualStimulationStop, current_time, 0)
		-- wait for BreakDuration seconds
		current_time = current_time + breakDuration
	end

	box:send_stimulation(1, Stim_ExperimentStop, current_time, 0)

	box:sleep()
end


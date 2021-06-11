board=nice_nano
shield=kyria
zmk_image=zmkfirmware/zmk-dev-arm:2.4
zmk=${PWD}/zmk
uf2=${PWD}/uf2
device=/dev/disk/by-label/NICENANO
mount=/media/${USER}/NICENANO
config=${PWD}/config

define build
	cd ${HOME}/src/zmk/app ;\
	west build --pristine -b "${board}" \
		-- -DSHIELD="$(1)" -DZMK_CONFIG="${config}" \
		&& cp build/zephyr/zmk.uf2 ${uf2}/$(1).uf2

endef

# define build
# 	docker run --rm \
# 		-v "${zmk}:/zmk" -v "${PWD}/config:/zmk-config" -v "${uf2}:/uf2" \
# 		-w /zmk/app "${zmk_image}" sh -c '\
# 			west build --pristine -b "${board}" \
# 			-- -DSHIELD="$(1)" -DZMK_CONFIG="/zmk-config" \
# 			&& cp build/zephyr/zmk.uf2 /uf2/$(1).uf2'
# endef

define flash
	@ printf "\nWaiting for a nice!nano to appear\n"
	@ while [ ! -b ${device} ]; do sleep 1; printf "."; done
	@ findmnt "${device}" || udisksctl mount --block-device ${device}
	@ printf "\nWaiting for $(1) ${board} to appear at ${mount} ."
	@ while [ ! -f "${mount}/current.uf2" ]; do sleep 1; printf "."; done
	@ printf "\n";
	cp -av "${uf2}/$(1).uf2" "${mount}/"
endef

default: flash-left

dirs:
	@mkdir -p uf2

.PHONY: uf2
uf2: dirs
	$(call build,${shield}_left)
	$(call build,${shield}_right)

.PHONY: flash-left
_flash-left: dirs
	$(call flash,${shield}_left)

.PHONY: flash-left
flash-left: dirs
	$(call build,${shield}_left)
	$(call flash,${shield}_left)

.PHONY: flash-right
flash-right: dirs
	$(call build,${shield}_right)
	$(call flash,${shield}_right)

# .PHONY: zmk
# zmk:
# 	docker run --rm -h make.zmk -w /zmk -v "${zmk}:/zmk" "${zmk_image}" sh -c '\
# 		git clone https://github.com/zmkfirmware/zmk .; \
# 		git remote add -ft macros okke-formsa https://github.com/okke-formsma/zmk; \
# 		git remote add -ft cradio-v2 davidphilipbarr https://github.com/davidphilipbarr/zmk; \
# 		git merge okke-formsa/macros --no-edit; \
# 		git merge davidphilipbarr/cradio-v2 --no-edit; \
# 		west init -l app; \
# 		west update;'

.PHONY: clean
clean:
	# sudo rm -rf "${uf2}" "${zmk}"
	sudo rm -rf "${uf2}"

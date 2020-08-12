FROM ubuntu

ARG USERNAME=vnt
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Avoid warnings by switching to noninteractive
ENV DEBIAN_FRONTEND=noninteractive

# Configure apt and install packages, change mirror
RUN sed -i -e 's/\w\+\.ubuntu\.com/mirrors.aliyun.com/g' /etc/apt/sources.list \
    && apt-get update \
    #
    # Install C++ tools
    && apt-get -y install build-essential cmake cppcheck valgrind python dkms\
    #
    # [Optional] Update UID/GID if needed
    # && if [ "$USER_GID" != "1000" ] || [ "$USER_UID" != "1000" ]; then \
    #     groupmod --gid $USER_GID $USERNAME \
    #     && usermod --uid $USER_UID --gid $USER_GID $USERNAME \
    #     && chown -R $USER_UID:$USER_GID /home/$USERNAME; \
    # fi \
    #需要增加sgx的配置过程
    && cd ./sgxsdk
    && ./build.sh
    #[optional] 需要增加sgxssl的配置过程
    && cd ../openssl
    && make SGX_MODE=SIM all
    && make install
    
    && groupadd --gid $USER_GID $USERNAME \
    && useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME \

    #
    # Clean up
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

# Switch back to dialog for any ad-hoc use of apt-get
ENV DEBIAN_FRONTEND=dialog

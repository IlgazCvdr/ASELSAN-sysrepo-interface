from ncclient import manager
from lxml import etree

# Replace with your Netopeer server details
host = 'ilgaz-ThinkCentre-neo-50t-Gen-4'
port = 830
username = 'ilgaz'
password = 'H2Oiswater!'

def create_network_interface(interface_name):
    rpc_request = f'''
    <create-network-interface xmlns="http://example.com/aselsan-network-settings">
        <interface-name>{interface_name}</interface-name>
    </create-network-interface>
    '''

    with manager.connect(
        host=host,
        port=port,
        username=username,
        password=password,
        hostkey_verify=False
    ) as m:
        try:
            response = m.dispatch(etree.fromstring(rpc_request))
            print("RPC response:", response)
        except Exception as e:
            print(f"Error sending RPC: {str(e)}")

def set_ip_settings(interface_name, ip_address):
    rpc_request = f'''
    <set-ip-settings xmlns="http://example.com/aselsan-network-settings">
        <interface-name>{interface_name}</interface-name>
        <ip-address>{ip_address}</ip-address>
    </set-ip-settings>
    '''
    print("RPC Request:")
    print(rpc_request)  # Print the RPC request for debugging

    with manager.connect(
        host=host,
        port=port,
        username=username,
        password=password,
        hostkey_verify=False
    ) as m:
        try:
            response = m.dispatch(etree.fromstring(rpc_request))
            print("RPC response:", response)
        except Exception as e:
            print(f"Error sending RPC: {str(e)}")


if __name__ == "__main__":
    # Example usage:
    create_network_interface("deneme")
    set_ip_settings("deneme", "192.111.55.66")
